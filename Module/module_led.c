/**
 *********************************************************************
 * @file    module_led.c
 * @brief   LED 控制业务模块实现
 * @note    支持手动模式和闪烁模式
 *          闪烁模式下通过 Module_LED_Process() 周期翻转
 *          所有 LED 输出最终通过 BSP_LED_Set() 完成
 *********************************************************************
 */
#include "module_led.h"

/* ---- 模块状态 ---- */
static LedMode  s_mode = LED_MODE_MANUAL;     /* 当前模式 */
static LedState s_led_state = LED_OFF;        /* LED 功能状态 */
static uint16_t s_blink_period = 500;         /* 闪烁周期（毫秒） */
static uint32_t s_blink_tick = 0;             /* 闪烁计时 */

/**
 * @brief  初始化 LED 模块
 */
void Module_LED_Init(void)
{
    s_mode = LED_MODE_MANUAL;
    s_led_state = LED_OFF;
    s_blink_period = 500;
    s_blink_tick = 0;

    BSP_LED_Init();
    BSP_LED_Set(LED_OFF);
}

/**
 * @brief  设置 LED 工作模式
 */
void Module_LED_SetMode(LedMode mode)
{
    if (mode == s_mode) return;

    if (mode == LED_MODE_BLINK) {
        /* 进入闪烁模式：重置计时，LED 初始为灭 */
        s_blink_tick = 0;
    }
    /* 如果切换到手动模式，保持当前 LED 状态不变 */

    s_mode = mode;
}

/**
 * @brief  获取当前模式
 */
LedMode Module_LED_GetMode(void)
{
    return s_mode;
}

/**
 * @brief  设置 LED 状态
 * @note   如果当前在闪烁模式，调用此函数会切换到手动模式
 */
void Module_LED_SetState(LedState state)
{
    if (s_mode != LED_MODE_MANUAL) {
        s_mode = LED_MODE_MANUAL;
    }
    s_led_state = state;
    BSP_LED_Set(state);
}

/**
 * @brief  翻转 LED
 * @note   如果当前在闪烁模式，调用此函数会切换到手动模式
 */
void Module_LED_Toggle(void)
{
    if (s_led_state == LED_ON) {
        Module_LED_SetState(LED_OFF);
    } else {
        Module_LED_SetState(LED_ON);
    }
}

/**
 * @brief  获取 LED 状态
 */
LedState Module_LED_GetState(void)
{
    return s_led_state;
}

/**
 * @brief  设置闪烁周期
 * @param  period_ms  周期（毫秒）
 * @note   范围限制在 100~5000ms，防止过快或过慢
 */
void Module_LED_SetBlinkPeriod(uint16_t period_ms)
{
    if (period_ms < 100)  period_ms = 100;
    if (period_ms > 5000) period_ms = 5000;
    s_blink_period = period_ms;
}

/**
 * @brief  获取闪烁周期
 */
uint16_t Module_LED_GetBlinkPeriod(void)
{
    return s_blink_period;
}

/**
 * @brief  LED 模块周期处理
 * @note   建议 10ms 调用一次，使用 HAL_GetTick() 作为时基
 *         闪烁模式下按周期翻转 LED
 */
void Module_LED_Process(void)
{
    static uint32_t last_tick = 0;
    uint32_t now = HAL_GetTick();
    uint32_t delta;

    if (last_tick == 0) {
        last_tick = now;
        return;
    }

    delta = now - last_tick;
    last_tick = now;

    if (s_mode == LED_MODE_BLINK) {
        s_blink_tick += delta;
        if (s_blink_tick >= s_blink_period) {
            s_blink_tick = 0;
            /* 翻转 LED */
            if (s_led_state == LED_ON) {
                s_led_state = LED_OFF;
            } else {
                s_led_state = LED_ON;
            }
            BSP_LED_Set(s_led_state);
        }
    }
}
