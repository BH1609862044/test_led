/**
 *********************************************************************
 * @file    module_display.c
 * @brief   OLED 显示模块实现（多级菜单）
 * @note    5 个页面：首页 / LED控制 / 闪烁参数 / 系统信息 / 命令说明
 *          KEY1 短按 = 下一页；KEY1 长按 = 回首页
 *          KEY2 短按 = 加/确认；KEY3 短按 = 减/返回
 *          KEY2/KEY3 长按连续触发 = 快速增减
 *********************************************************************
 */
#include "module_display.h"
#include "device_oled.h"
#include "module_led.h"
#include "module_params.h"
#include "module_sysinfo.h"
#include "component_key.h"
#include <stdio.h>
#include <string.h>

/* ---- 私有变量 ---- */
static PageType s_current_page = PAGE_HOME;
static uint8_t  s_page_dirty = 1;  /* 页面脏标记：1 = 需要全刷 */

/* 各页面的选中项 / 编辑值 */
static int s_led_cursor = 0;       /* LED 控制页光标：0=模式, 1=状态 */

/* 错误提示 */
static char s_error_buf[17];
static uint8_t s_error_show = 0;
static uint32_t s_error_tick = 0;

/* 页码指示缓存（避免重复刷新） */
static LedMode s_last_mode = (LedMode)0xFF;
static LedState s_last_led_state = (LedState)0xFF;
static uint32_t s_last_count = 0xFFFFFFFF;
static uint32_t s_last_uptime = 0xFFFFFFFF;
static uint8_t  s_last_brightness = 0xFF;   /* 呼吸模式亮度缓存 */

/* ---- 私有函数声明 ---- */
static void draw_home_page(void);
static void draw_led_ctrl_page(void);
static void draw_blink_param_page(void);
static void draw_fast_param_page(void);
static void draw_breath_param_page(void);
static void draw_sysinfo_page(void);
static void draw_cmd_help_page(void);
static void draw_page_indicator(void);
static void refresh_home_dynamic(void);
static void refresh_sysinfo_dynamic(void);
static void show_error_check(void);

/**
 * @brief  初始化
 */
void Module_Display_Init(void)
{
    Device_OLED_Init();
    Device_OLED_Clear();
    s_current_page = PAGE_HOME;
    s_page_dirty = 1;
    s_error_show = 0;
    s_led_cursor = 0;
}

/**
 * @brief  显示启动页
 */
void Module_Display_ShowBootPage(void)
{
    Device_OLED_Clear();
    Device_OLED_ShowString(0, 0, "STM32 CONTROL");
    Device_OLED_ShowString(0, 2, "  Booting...  ");
    Device_OLED_ShowString(0, 4, " v1.2.3 ");
    Device_OLED_Refresh();
    s_current_page = PAGE_BOOT;
}

/**
 * @brief  切换到首页
 */
void Module_Display_ShowHomePage(void)
{
    s_current_page = PAGE_HOME;
    s_page_dirty = 1;
}

/**
 * @brief  菜单按键输入
 */
void Module_Display_MenuKey(MenuKeyOp op)
{
    if (op == MENU_KEY_HOME) {
        if (s_current_page != PAGE_HOME) {
            s_current_page = PAGE_HOME;
            s_page_dirty = 1;
        }
        return;
    }

    switch (s_current_page) {

    /* ---- 首页 ---- */
    case PAGE_HOME:
        if (op == MENU_KEY_NEXT) {
            s_current_page = PAGE_LED_CTRL;
            s_led_cursor = 0;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_PREV) {
            /* 已经在首页，什么都不做 */
        }
        break;

    /* ---- LED 控制页 ---- */
    case PAGE_LED_CTRL:
        if (op == MENU_KEY_NEXT) {
            s_current_page = PAGE_BLINK_PARAM;
                    s_page_dirty = 1;
        } else if (op == MENU_KEY_PREV) {
            s_current_page = PAGE_HOME;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_INC) {
            if (s_led_cursor == 0) {
                /* 切换模式 */
                LedMode m = Module_LED_GetMode();
                m = (LedMode)((m + 1) % 4);  /* 0/1/2/3 循环 */
                Module_LED_SetMode(m);
                Module_Params_SetLedMode(m);
            } else {
                /* 切换 LED 状态 */
                Module_LED_Toggle();
                Module_Params_SetLedState((uint8_t)Module_LED_GetState());
            }
            s_page_dirty = 1;
        } else if (op == MENU_KEY_DEC) {
            /* 上下移动光标 */
            s_led_cursor = (s_led_cursor == 0) ? 1 : 0;
            s_page_dirty = 1;
        }
        break;

    /* ---- P3：慢闪周期（KEY2 加 / KEY3 减，10ms 一档） ---- */
    case PAGE_BLINK_PARAM:
        if (op == MENU_KEY_NEXT) {
            s_current_page = PAGE_FAST_PARAM;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_PREV) {
            s_current_page = PAGE_LED_CTRL;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_INC || op == MENU_KEY_INC_HOLD) {
            uint16_t step = (op == MENU_KEY_INC_HOLD) ? 50 : 10;
            uint16_t p = Module_LED_GetBlinkPeriod();
            if ((uint32_t)p + step <= PARAM_BLINK_PERIOD_MAX) {
                p = (uint16_t)(p + step);
            }
            Module_LED_SetBlinkPeriod(p);
            Module_Params_SetBlinkPeriod(p);
            s_page_dirty = 1;
        } else if (op == MENU_KEY_DEC || op == MENU_KEY_DEC_HOLD) {
            uint16_t step = (op == MENU_KEY_DEC_HOLD) ? 50 : 10;
            uint16_t p = Module_LED_GetBlinkPeriod();
            if (p > step && (uint32_t)p - step >= PARAM_BLINK_PERIOD_MIN) {
                p = (uint16_t)(p - step);
            } else {
                p = PARAM_BLINK_PERIOD_MIN;
            }
            Module_LED_SetBlinkPeriod(p);
            Module_Params_SetBlinkPeriod(p);
            s_page_dirty = 1;
        }
        break;

    /* ---- P4：快闪周期 ---- */
    case PAGE_FAST_PARAM:
        if (op == MENU_KEY_NEXT) {
            s_current_page = PAGE_BREATH_PARAM;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_PREV) {
            s_current_page = PAGE_BLINK_PARAM;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_INC || op == MENU_KEY_INC_HOLD) {
            uint16_t step = (op == MENU_KEY_INC_HOLD) ? 50 : 10;
            uint16_t p = Module_LED_GetFastPeriod();
            if ((uint32_t)p + step <= PARAM_FAST_PERIOD_MAX) {
                p = (uint16_t)(p + step);
            }
            Module_LED_SetFastPeriod(p);
            Module_Params_SetFastPeriod(p);
            s_page_dirty = 1;
        } else if (op == MENU_KEY_DEC || op == MENU_KEY_DEC_HOLD) {
            uint16_t step = (op == MENU_KEY_DEC_HOLD) ? 50 : 10;
            uint16_t p = Module_LED_GetFastPeriod();
            if (p > step && (uint32_t)p - step >= PARAM_FAST_PERIOD_MIN) {
                p = (uint16_t)(p - step);
            } else {
                p = PARAM_FAST_PERIOD_MIN;
            }
            Module_LED_SetFastPeriod(p);
            Module_Params_SetFastPeriod(p);
            s_page_dirty = 1;
        }
        break;

    /* ---- P5：呼吸周期（100ms 一档） ---- */
    case PAGE_BREATH_PARAM:
        if (op == MENU_KEY_NEXT) {
            s_current_page = PAGE_SYSINFO;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_PREV) {
            s_current_page = PAGE_FAST_PARAM;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_INC || op == MENU_KEY_INC_HOLD) {
            uint16_t step = (op == MENU_KEY_INC_HOLD) ? 500 : 100;
            uint16_t p = Module_LED_GetBreathPeriod();
            if ((uint32_t)p + step <= PARAM_BREATH_PERIOD_MAX) {
                p = (uint16_t)(p + step);
            }
            Module_LED_SetBreathPeriod(p);
            Module_Params_SetBreathPeriod(p);
            s_page_dirty = 1;
        } else if (op == MENU_KEY_DEC || op == MENU_KEY_DEC_HOLD) {
            uint16_t step = (op == MENU_KEY_DEC_HOLD) ? 500 : 100;
            uint16_t p = Module_LED_GetBreathPeriod();
            if (p > step && (uint32_t)p - step >= PARAM_BREATH_PERIOD_MIN) {
                p = (uint16_t)(p - step);
            } else {
                p = PARAM_BREATH_PERIOD_MIN;
            }
            Module_LED_SetBreathPeriod(p);
            Module_Params_SetBreathPeriod(p);
            s_page_dirty = 1;
        }
        break;
    /* ---- 系统信息页 ---- */
    case PAGE_SYSINFO:
        if (op == MENU_KEY_NEXT) {
            s_current_page = PAGE_CMD_HELP;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_PREV) {
            s_current_page = PAGE_BREATH_PARAM;
            s_page_dirty = 1;
        }
        break;

    /* ---- 命令说明页 ---- */
    case PAGE_CMD_HELP:
        if (op == MENU_KEY_NEXT) {
            /* 循环回到首页 */
            s_current_page = PAGE_HOME;
            s_page_dirty = 1;
        } else if (op == MENU_KEY_PREV) {
            s_current_page = PAGE_SYSINFO;
            s_page_dirty = 1;
        }
        break;

    default:
        break;
    }
}

/**
 * @brief  获取当前页面
 */
PageType Module_Display_GetCurrentPage(void)
{
    return s_current_page;
}

/**
 * @brief  周期刷新（100ms 一次）
 */
void Module_Display_Process(void)
{
    /* 页面切换时全刷 */
    if (s_page_dirty) {
        s_page_dirty = 0;
        Device_OLED_Clear();
        switch (s_current_page) {
        case PAGE_HOME:        draw_home_page(); break;
        case PAGE_LED_CTRL:    draw_led_ctrl_page(); break;
        case PAGE_BLINK_PARAM:  draw_blink_param_page(); break;
        case PAGE_FAST_PARAM:   draw_fast_param_page(); break;
        case PAGE_BREATH_PARAM: draw_breath_param_page(); break;
        case PAGE_SYSINFO:     draw_sysinfo_page(); break;
        case PAGE_CMD_HELP:    draw_cmd_help_page(); break;
        default: break;
        }
        draw_page_indicator();
        Device_OLED_Refresh();
        return;
    }

    /* 动态内容局部刷新 */
    switch (s_current_page) {
    case PAGE_HOME:    refresh_home_dynamic(); break;
    case PAGE_SYSINFO: refresh_sysinfo_dynamic(); break;
    default: break;
    }

    /* 错误提示超时检查 */
    show_error_check();
}

/* ========== 各页面绘制 ========== */

/**
 * @brief  首页
 */
static void draw_home_page(void)
{
    char buf[16];

    Device_OLED_ShowString(0, 0, "STM32 CONTROL");

    /* 模式 */
    Device_OLED_ShowString(0, 2, "MODE: ");
    Device_OLED_ShowString(6*6, 2, Module_LED_GetModeName(Module_LED_GetMode()));
    s_last_mode = Module_LED_GetMode();

    /* LED 状态（呼吸模式下显示实时亮度百分比） */
    Device_OLED_ShowString(0, 3, "LED: ");
    if (Module_LED_GetMode() == LED_MODE_BREATH) {
        snprintf(buf, sizeof(buf), "%3u%%", (unsigned)Module_LED_GetBrightness());
    } else {
        snprintf(buf, sizeof(buf), "%s", (Module_LED_GetState() == LED_ON) ? "ON  " : "OFF ");
    }
    Device_OLED_ShowString(5*6, 3, buf);
    s_last_led_state = Module_LED_GetState();
    s_last_brightness = Module_LED_GetBrightness();

    /* 按键次数 */
    snprintf(buf, sizeof(buf), "COUNT:%04lu", (unsigned long)Key_GetEventCount());
    Device_OLED_ShowString(0, 5, buf);
    s_last_count = Key_GetEventCount();

    /* 运行时间 */
    snprintf(buf, sizeof(buf), "TIME:%05lus", (unsigned long)Module_SysInfo_GetUptime());
    Device_OLED_ShowString(0, 6, buf);
    s_last_uptime = Module_SysInfo_GetUptime();
}

/**
 * @brief  LED 控制页
 */
static void draw_led_ctrl_page(void)
{
    Device_OLED_ShowString(0, 0, ">> LED CTRL <<");

    /* 第 2 行：模式选择 */
    if (s_led_cursor == 0) {
        Device_OLED_ShowString(0, 2, "> MODE: ");
    } else {
        Device_OLED_ShowString(0, 2, "  MODE: ");
    }
    Device_OLED_ShowString(8*6, 2, Module_LED_GetModeName(Module_LED_GetMode()));

    /* 第 4 行：LED 状态 */
    if (s_led_cursor == 1) {
        Device_OLED_ShowString(0, 4, "> LED : ");
    } else {
        Device_OLED_ShowString(0, 4, "  LED : ");
    }
    Device_OLED_ShowString(8*6, 4, (Module_LED_GetState() == LED_ON) ? "ON " : "OFF");

    Device_OLED_ShowString(0, 6, "K2=SET  K3=MOVE");
}

/**
 * @brief  闪烁参数页
 */
static void draw_blink_param_page(void)
{
    char buf[16];

    Device_OLED_ShowString(0, 0, ">> BLINK PARA <<");
    Device_OLED_ShowString(0, 2, "PERIOD: ");
    snprintf(buf, sizeof(buf), "%5ums", (unsigned)Module_LED_GetBlinkPeriod());
    Device_OLED_ShowString(8*6, 2, buf);

    Device_OLED_ShowString(0, 4, "K2=+  K3=-");
    Device_OLED_ShowString(0, 5, "hold = fast");
    Device_OLED_ShowString(0, 6, "100-5000ms");
}

/**
 * @brief  P4 快闪周期
 */
static void draw_fast_param_page(void)
{
    char buf[16];

    Device_OLED_ShowString(0, 0, ">> FAST PARA <<");
    Device_OLED_ShowString(0, 2, "PERIOD: ");
    snprintf(buf, sizeof(buf), "%5ums", (unsigned)Module_LED_GetFastPeriod());
    Device_OLED_ShowString(8*6, 2, buf);

    Device_OLED_ShowString(0, 4, "K2=+  K3=-");
    Device_OLED_ShowString(0, 5, "hold = fast");
    Device_OLED_ShowString(0, 6, "100-5000ms");
}

/**
 * @brief  P5 呼吸周期
 */
static void draw_breath_param_page(void)
{
    char buf[16];

    Device_OLED_ShowString(0, 0, ">> BREATH PARA <<");
    Device_OLED_ShowString(0, 2, "PERIOD: ");
    snprintf(buf, sizeof(buf), "%5ums", (unsigned)Module_LED_GetBreathPeriod());
    Device_OLED_ShowString(8*6, 2, buf);

    Device_OLED_ShowString(0, 4, "K2=+  K3=-");
    Device_OLED_ShowString(0, 5, "hold = fast");
    Device_OLED_ShowString(0, 6, "1000-10000ms");
}

/**
 * @brief  系统信息页
 */
static void draw_sysinfo_page(void)
{
    char buf[16];

    Device_OLED_ShowString(0, 0, ">> SYS INFO <<");

    Device_OLED_ShowString(0, 2, "VER: ");
    Device_OLED_ShowString(5*6, 2, Module_SysInfo_GetSwVersion());

    snprintf(buf, sizeof(buf), "UP:%05lus", (unsigned long)Module_SysInfo_GetUptime());
    Device_OLED_ShowString(0, 3, buf);
    s_last_uptime = Module_SysInfo_GetUptime();

    snprintf(buf, sizeof(buf), "KEY:%04lu", (unsigned long)Key_GetEventCount());
    Device_OLED_ShowString(0, 4, buf);

    snprintf(buf, sizeof(buf), "CMD OK:%lu", (unsigned long)Module_SysInfo_GetCmdOkCount());
    Device_OLED_ShowString(0, 5, buf);

    snprintf(buf, sizeof(buf), "ERR:%lu", (unsigned long)Module_SysInfo_GetCmdErrCount());
    Device_OLED_ShowString(0, 6, buf);
}

/**
 * @brief  命令说明页
 */
static void draw_cmd_help_page(void)
{
    Device_OLED_ShowString(0, 0, ">> CMD HELP <<");
    Device_OLED_ShowString(0, 2, "GET STATUS");
    Device_OLED_ShowString(0, 3, "GET PARAM");
    Device_OLED_ShowString(0, 4, "SET MODE 0/1/2/3");
    Device_OLED_ShowString(0, 5, "SET PERIOD N");
    Device_OLED_ShowString(0, 6, "SAVE / RESET");
    Device_OLED_ShowString(0, 7, "See UART HELP");
}

/**
 * @brief  绘制页面指示器（底部显示页码）
 */
static void draw_page_indicator(void)
{
    char buf[8];
    if (s_current_page >= PAGE_HOME && s_current_page <= PAGE_CMD_HELP) {
        snprintf(buf, sizeof(buf), "P%u/7", (unsigned)(s_current_page - PAGE_HOME + 1));
        Device_OLED_ShowString(16*6 - 4*6, 7, buf);
    }
}

/* ========== 动态内容局部刷新 ========== */

/**
 * @brief  首页动态内容刷新
 */
static void refresh_home_dynamic(void)
{
    char buf[16];

    /* 模式变化 */
    if (s_last_mode != Module_LED_GetMode()) {
        Device_OLED_ShowString(6*6, 2, "     ");
        Device_OLED_ShowString(6*6, 2, Module_LED_GetModeName(Module_LED_GetMode()));
        s_last_mode = Module_LED_GetMode();
        s_last_led_state = (LedState)0xFF;   /* 强制下一拍重画 LED 行 */
        s_last_brightness = 0xFF;
        Device_OLED_Refresh();
    }

    /* LED 状态变化（呼吸模式下显示实时亮度） */
    if (Module_LED_GetMode() == LED_MODE_BREATH) {
        if (s_last_brightness != Module_LED_GetBrightness()) {
            snprintf(buf, sizeof(buf), "%3u%%", (unsigned)Module_LED_GetBrightness());
            Device_OLED_ShowString(5*6, 3, "    ");
            Device_OLED_ShowString(5*6, 3, buf);
            s_last_brightness = Module_LED_GetBrightness();
            Device_OLED_Refresh();
        }
    } else if (s_last_led_state != Module_LED_GetState()) {
        Device_OLED_ShowString(5*6, 3, "    ");
        Device_OLED_ShowString(5*6, 3, (Module_LED_GetState() == LED_ON) ? "ON  " : "OFF ");
        s_last_led_state = Module_LED_GetState();
        Device_OLED_Refresh();
    }

    /* 按键次数变化 */
    if (s_last_count != Key_GetEventCount()) {
        snprintf(buf, sizeof(buf), "%04lu", (unsigned long)Key_GetEventCount());
        Device_OLED_ShowString(6*6, 5, buf);
        s_last_count = Key_GetEventCount();
        Device_OLED_Refresh();
    }

    /* 运行时间（每秒变化） */
    if (s_last_uptime != Module_SysInfo_GetUptime()) {
        snprintf(buf, sizeof(buf), "%05lus", (unsigned long)Module_SysInfo_GetUptime());
        Device_OLED_ShowString(5*6, 6, buf);
        s_last_uptime = Module_SysInfo_GetUptime();
        Device_OLED_Refresh();
    }
}

/**
 * @brief  系统信息页动态刷新
 */
static void refresh_sysinfo_dynamic(void)
{
    char buf[16];

    if (s_last_uptime != Module_SysInfo_GetUptime()) {
        snprintf(buf, sizeof(buf), "%05lus", (unsigned long)Module_SysInfo_GetUptime());
        Device_OLED_ShowString(3*6, 3, buf);
        s_last_uptime = Module_SysInfo_GetUptime();
        Device_OLED_Refresh();
    }
}

/* ========== 错误提示 ========== */

/**
 * @brief  显示错误提示
 */
void Module_Display_ShowError(const char *err_str)
{
    if (err_str == NULL) return;

    strncpy(s_error_buf, err_str, sizeof(s_error_buf) - 1);
    s_error_buf[sizeof(s_error_buf) - 1] = '\0';

    /* 在屏幕底部显示 */
    Device_OLED_ShowString(0, 7, "                ");
    Device_OLED_ShowString(0, 7, "ERR:");
    Device_OLED_ShowString(4*6, 7, s_error_buf);
    Device_OLED_Refresh();

    s_error_show = 1;
    s_error_tick = HAL_GetTick();
}

/**
 * @brief  错误提示超时自动清除
 */
static void show_error_check(void)
{
    if (!s_error_show) return;

    if (HAL_GetTick() - s_error_tick > 2000) {
        /* 2 秒后清除，重新绘制底部页码 */
        s_error_show = 0;
        s_page_dirty = 1;  /* 触发重绘 */
    }
}
