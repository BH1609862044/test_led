/**
 *********************************************************************
 * @file    app.c
 * @brief   应用层实现（提高题版本）
 * @note    负责系统启动自检、按键菜单事件分发、主循环调度
 *
 * 按键功能（提高题 - 菜单导航）：
 *   KEY1 短按   —— 下一页
 *   KEY1 长按   —— 回到首页
 *   KEY2 短按   —— 加 / 确认（当前页操作）
 *   KEY2 长按+  —— 连续加（调整参数时）
 *   KEY3 短按   —— 减 / 移动光标 / 返回
 *   KEY3 长按+  —— 连续减（调整参数时）
 *
 * 启动流程：
 *   1. 显示启动页
 *   2. LED 闪烁自检
 *   3. OLED 检测 + 串口输出版本信息
 *   4. 加载 Flash 参数
 *   5. 进入首页
 *********************************************************************
 */
#include "app.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_uart.h"
#include "device_oled.h"
#include "device_uart.h"
#include "component_key.h"
#include "module_led.h"
#include "module_cmd.h"
#include "module_display.h"
#include "module_params.h"
#include "module_sysinfo.h"
#include "task_scheduler.h"

#include <stdio.h>
#include <string.h>

/* ---- 启动时间戳 ---- */
static uint32_t s_boot_tick = 0;

/* ============================================================
 * 按键事件处理（菜单导航）
 * ============================================================ */

/**
 * @brief  按键事件回调
 * @param  key_id  按键编号 0/1/2
 * @param  event   事件类型（KeyEventType）
 */
void App_OnKeyEvent(uint8_t key_id, uint8_t event)
{
    MenuKeyOp op;

    /* 根据按键和事件类型映射到菜单操作 */
    if (key_id == 0) {           /* KEY1 */
        if (event == KEY_EVT_SHORT) {
            op = MENU_KEY_NEXT;     /* 短按 = 下一页 */
        } else if (event == KEY_EVT_LONG) {
            op = MENU_KEY_HOME;     /* 长按 = 回首页 */
        } else {
            return;
        }
    }
    else if (key_id == 1) {      /* KEY2 */
        if (event == KEY_EVT_SHORT) {
            op = MENU_KEY_INC;      /* 短按 = 加/确认 */
        } else if (event == KEY_EVT_REPEAT) {
            op = MENU_KEY_INC_HOLD; /* 连续触发 = 快速加 */
        } else {
            return;
        }
    }
    else if (key_id == 2) {      /* KEY3 */
        if (event == KEY_EVT_SHORT) {
            op = MENU_KEY_DEC;      /* 短按 = 减/移动 */
        } else if (event == KEY_EVT_REPEAT) {
            op = MENU_KEY_DEC_HOLD; /* 连续触发 = 快速减 */
        } else {
            return;
        }
    }
    else {
        return;
    }

    /* 分发给显示模块（菜单处理） */
    Module_Display_MenuKey(op);
}

/* ============================================================
 * 上电自检
 * ============================================================ */

/**
 * @brief  启动自检
 */
static void App_BootSelfTest(void)
{
    uint8_t i;
    char buf[48];

    /* 显示启动页 */
    Module_Display_ShowBootPage();

    /* LED 闪烁自检：闪 3 次 */
    for (i = 0; i < 3; i++) {
        BSP_LED_Set(LED_ON);
        HAL_Delay(200);
        BSP_LED_Set(LED_OFF);
        HAL_Delay(200);
    }

    /* 串口输出启动信息 */
    BSP_UART_SendString("\r\n==============================\r\n");
    BSP_UART_SendString("  ");
    BSP_UART_SendString(APP_PROJECT_NAME);
    BSP_UART_SendString("\r\n");
    BSP_UART_SendString("  Version: ");
    BSP_UART_SendString(APP_VERSION_STR);
    BSP_UART_SendString("\r\n");
    BSP_UART_SendString("  Baudrate: 115200\r\n");
    BSP_UART_SendString("  Type HELP for command list\r\n");
    BSP_UART_SendString("==============================\r\n");

    /* OLED 连接检测 */
    if (Device_OLED_TestConnection()) {
        BSP_UART_SendString("[OK]  OLED detected at 0x78\r\n");
    } else {
        BSP_UART_SendString("[ERR] OLED NOT found at 0x78!\r\n");
        BSP_UART_SendString("       Check wiring: SCL=PB6, SDA=PB7\r\n");
        Module_Display_ShowError("OLED ERR");
    }

    /* 参数加载结果 */
    snprintf(buf, sizeof(buf), "[INFO] Param: mode=%u period=%u\r\n",
             (unsigned)Module_Params_Get()->led_mode,
             (unsigned)Module_Params_Get()->blink_period_ms);
    BSP_UART_SendString(buf);

    /* 稍作停留 */
    HAL_Delay(800);

    /* 进入首页 */
    Module_Display_ShowHomePage();
}

/* ============================================================
 * 公共接口
 * ============================================================ */

/**
 * @brief  应用初始化
 * @note   按依赖顺序自底向上初始化各层
 */
void App_Init(void)
{
    /* 记录启动时间 */
    s_boot_tick = HAL_GetTick();

    /* ---- BSP 层 ---- */
    BSP_LED_Init();
    BSP_Key_Init();
    BSP_UART_Init();

    /* ---- Device 层 ---- */
    Device_UART_Init();
    /* OLED 在 Module_Display_Init 中初始化 */

    /* ---- Component 层 ---- */
    Key_Init(App_OnKeyEvent);           /* 注册按键事件回调 */
    Key_SetDebounceTime(20);            /* 20ms 消抖 */
    Key_SetLongPressTime(1000);         /* 1s 长按 */
    Key_SetRepeatInterval(200);         /* 200ms 连续触发 */

    /* ---- Module 层 ---- */
    Module_SysInfo_Init();              /* 系统信息 */
    Module_Params_Init();               /* 参数管理（从 Flash 加载） */
    Module_LED_Init();                  /* LED 控制 */

    /* 应用保存的参数到 LED 模块 */
    {
        const ParamsStruct *p = Module_Params_Get();
        Module_LED_SetBlinkPeriod(p->blink_period_ms);
        Module_LED_SetFastPeriod(p->fast_period_ms);
        Module_LED_SetMode((LedMode)p->led_mode);
        if (p->led_state == LED_ON) {
            Module_LED_SetState(LED_ON);
        }
    }

    Module_Cmd_Init();                  /* 串口命令 */
    Module_Display_Init();              /* OLED 显示（内部初始化 OLED） */

    /* ---- Task 层 ---- */
    Task_SchedulerInit();

    /* ---- 上电自检 ---- */
    App_BootSelfTest();
}

/**
 * @brief  应用主循环
 */
void App_Loop(void)
{
    Task_SchedulerRun();
}

/**
 * @brief  获取系统运行时间（秒）
 */
uint32_t App_GetUptimeSeconds(void)
{
    return (HAL_GetTick() - s_boot_tick) / 1000;
}
