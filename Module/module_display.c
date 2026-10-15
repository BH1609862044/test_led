/**
 *********************************************************************
 * @file    module_display.c
 * @brief   OLED 显示业务模块实现
 * @note    主界面布局：
 *          第0页：STM32 CONTROL     （标题）
 *          第1页：MODE: MANUAL       （当前模式）
 *          第2页：LED: OFF           （LED 状态）
 *          第3页：COUNT: 0000        （按键计数）
 *          第4页：TIME: 0000s        （运行时间）
 *
 *          显示数据从 Module_LED 等模块读取，不直接操作硬件
 *          通过 device_oled 接口输出
 *********************************************************************
 */
#include "module_display.h"
#include "device_oled.h"
#include "module_led.h"
#include "module_cmd.h"
#include "component_key.h"

/* ---- 状态 ---- */
static PageType s_current_page = PAGE_BOOT;
static uint8_t  s_need_refresh = 1;    /* 全屏刷新标志 */

/* 缓存上一次显示的值，用于只刷新变化行 */
static LedMode  s_last_mode = 0xFF;
static LedState s_last_led = 0xFF;
static uint32_t s_last_count = 0xFFFFFFFF;
static uint32_t s_last_seconds = 0xFFFFFFFF;

/**
 * @brief  显示模块初始化
 */
void Module_Display_Init(void)
{
    OLED_Init();
    OLED_Clear();
    s_current_page = PAGE_BOOT;
    s_need_refresh = 1;
    s_last_mode = 0xFF;
    s_last_led = 0xFF;
    s_last_count = 0xFFFFFFFF;
    s_last_seconds = 0xFFFFFFFF;
}

/**
 * @brief  显示启动页（上电自检界面）
 */
void Module_Display_ShowBootPage(void)
{
    OLED_Clear();
    OLED_ShowString(0, 20, "STM32 OLED");
    OLED_ShowString(1, 16, "LED Control");
    OLED_ShowString(2, 22, "Console");
    OLED_ShowString(4, 4,  "Version: v1.0");
    OLED_ShowString(6, 16, "Booting...");
    s_current_page = PAGE_BOOT;
    s_need_refresh = 1;
}

/**
 * @brief  切换到主界面
 */
void Module_Display_ShowMainPage(void)
{
    OLED_Clear();
    s_current_page = PAGE_MAIN;
    s_need_refresh = 1;
    s_last_mode = 0xFF;
    s_last_led = 0xFF;
    s_last_count = 0xFFFFFFFF;
    s_last_seconds = 0xFFFFFFFF;

    /* 静态标题行 */
    OLED_ShowString(0, 12, "STM32 CONTROL");

    /* 立即刷新一次完整界面 */
    Module_Display_RefreshMain();
}

/**
 * @brief  刷新主界面（只刷新变化的行）
 */
void Module_Display_RefreshMain(void)
{
    if (s_current_page != PAGE_MAIN) return;

    /* --- 模式行 --- */
    {
        LedMode mode = Module_LED_GetMode();
        if (mode != s_last_mode || s_need_refresh) {
            OLED_ShowString(2, 0, "                ");  /* 清行 */
            if (mode == LED_MODE_MANUAL) {
                OLED_ShowString(2, 0, "MODE: MANUAL");
            } else {
                OLED_ShowString(2, 0, "MODE: BLINK");
            }
            s_last_mode = mode;
        }
    }

    /* --- LED 状态行 --- */
    {
        LedState state = Module_LED_GetState();
        if (state != s_last_led || s_need_refresh) {
            OLED_ShowString(3, 0, "                ");  /* 清行 */
            if (state == LED_ON) {
                OLED_ShowString(3, 0, "LED: ON");
            } else {
                OLED_ShowString(3, 0, "LED: OFF");
            }
            s_last_led = state;
        }
    }

    /* --- 按键计数行 --- */
    {
        uint32_t count = Key_GetEventCount();
        /* 注意：Key_GetEventCount 不会清零，但我们只显示增量值 */
        if (count != s_last_count || s_need_refresh) {
            OLED_ShowString(4, 0, "COUNT:     ");
            OLED_ShowNum(4, 6*6, count, 4);
            s_last_count = count;
        }
    }

    s_need_refresh = 0;
}

/**
 * @brief  显示模块周期处理（100ms 调用一次）
 * @note   刷新运行时间等动态信息
 */
void Module_Display_Process(void)
{
    static uint32_t tick_100ms = 0;
    uint32_t now = HAL_GetTick();
    uint32_t seconds;

    /* 每 100ms 处理一次 */
    if (now - tick_100ms < 100) return;
    tick_100ms = now;

    if (s_current_page != PAGE_MAIN) return;

    /* 刷新运行时间（秒）*/
    seconds = now / 1000;
    if (seconds != s_last_seconds) {
        OLED_ShowString(5, 0, "TIME:       ");
        OLED_ShowNum(5, 5*6, seconds, 5);
        OLED_ShowString(5, 10*6, "s");
        s_last_seconds = seconds;
    }

    /* 定期刷新模式和 LED 状态，防止意外错乱 */
    Module_Display_RefreshMain();
}

/**
 * @brief  显示错误信息（临时显示在第 7 页）
 */
void Module_Display_ShowError(const char *err_str)
{
    if (err_str == 0) return;
    OLED_ShowString(7, 0, "                ");  /* 清行 */
    OLED_ShowString(7, 0, err_str);
}
