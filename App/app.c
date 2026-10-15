/**
 *********************************************************************
 * @file    app.c
 * @brief   应用层实现
 * @note    负责系统启动自检、按键事件分发、主循环调度
 *
 *          基础题按键功能：
 *            KEY1 —— 切换工作模式（手动 <-> 闪烁）
 *            KEY2 —— 翻转 LED（手动模式下）
 *            KEY3 —— 手动模式下点亮/熄灭 LED
 *
 *          启动流程：
 *            1. 显示启动页
 *            2. LED 闪烁自检
 *            3. 串口输出启动信息
 *            4. 延时后进入主界面
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
#include "task_scheduler.h"

#include <stdio.h>
#include <string.h>

/* ---- 启动时间戳 ---- */
static uint32_t s_boot_tick = 0;

/* ============================================================
 * 按键事件处理
 * ============================================================ */

/**
 * @brief  按键事件回调
 * @param  key_id  按键编号 0/1/2
 * @param  event   事件类型（1 = 短按）
 * @note   由 Component/Key 在 Key_Scan 中触发
 *          在此处分发到具体业务逻辑
 */
void App_OnKeyEvent(uint8_t key_id, uint8_t event)
{
    if (event != 1) return;  /* 只处理短按事件 */

    switch (key_id) {
    case 0:  /* KEY1 —— 切换工作模式 */
        {
            LedMode mode = Module_LED_GetMode();
            if (mode == LED_MODE_MANUAL) {
                Module_LED_SetMode(LED_MODE_BLINK);
            } else {
                Module_LED_SetMode(LED_MODE_MANUAL);
            }
            /* 触发显示刷新 */
            Module_Display_RefreshMain();
        }
        break;

    case 1:  /* KEY2 —— 翻转 LED（手动模式） */
        {
            Module_LED_Toggle();  /* 闪烁模式下调用会自动切换到手动 */
            Module_Display_RefreshMain();
        }
        break;

    case 2:  /* KEY3 —— 切换 LED 开关（手动模式下点亮/熄灭） */
        {
            /* 基础题：KEY3 翻转 LED 状态（与 KEY2 功能略有区分即可）
             * 也可实现为：KEY3 确认当前模式/参数
             * 这里实现为在闪烁模式下调整闪烁周期 +50ms */
            if (Module_LED_GetMode() == LED_MODE_BLINK) {
                uint16_t period = Module_LED_GetBlinkPeriod();
                period += 100;
                if (period > 2000) period = 200;
                Module_LED_SetBlinkPeriod(period);
            } else {
                /* 手动模式下切换 ON/OFF */
                Module_LED_Toggle();
            }
            Module_Display_RefreshMain();
        }
        break;

    default:
        break;
    }
}

/* ============================================================
 * 上电自检
 * ============================================================ */

/**
 * @brief  启动自检
 * @note    显示启动页、LED 闪烁、串口输出版本信息
 */
static void App_BootSelfTest(void)
{
    uint8_t i;

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
    Device_UART_SendString("\r\n==============================\r\n");
    Device_UART_SendString("  ");
    Device_UART_SendString(APP_PROJECT_NAME);
    Device_UART_SendString("\r\n");
    Device_UART_SendString("  Version: ");
    Device_UART_SendString(APP_VERSION_STR);
    Device_UART_SendString("\r\n");
    Device_UART_SendString("  Baudrate: 115200\r\n");
    Device_UART_SendString("  Type HELP for command list\r\n");
    Device_UART_SendString("==============================\r\n");

    /* 稍作停留，显示启动页 */
    HAL_Delay(1000);

    /* 进入主界面 */
    Module_Display_ShowMainPage();
}

/* ============================================================
 * 公共接口
 * ============================================================ */

/**
 * @brief  应用初始化
 * @note   按依赖顺序自底向上初始化各层
 *         调用时机：main.c 中 MX_xxx_Init() 全部完成之后
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
    /* OLED 在 Module_Display_Init 中初始化，因为它是设备的封装 */

    /* ---- Component 层 ---- */
    Key_Init(App_OnKeyEvent);     /* 注册按键事件回调 */
    Key_SetDebounceTime(20);      /* 20ms 消抖 */

    /* ---- Module 层 ---- */
    Module_LED_Init();
    Module_Cmd_Init();
    Module_Display_Init();        /* 内部调用 OLED_Init() */

    /* ---- Task 层 ---- */
    Task_SchedulerInit();

    /* ---- 上电自检 ---- */
    App_BootSelfTest();
}

/**
 * @brief  应用主循环
 * @note   在 main.c 的 while(1) 中调用
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
