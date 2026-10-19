/**
 *********************************************************************
 * @file    module_led.c
 * @brief   LED 控制业务模块实现
 * @note    支持手动模式、固定闪烁、快速闪烁 3 种模式
 *          所有 LED 输出最终通过 BSP_LED_Set() 完成
 *********************************************************************
 */
#include "module_led.h"

/* ---- 模块状态 ---- */
static LedMode  s_mode = LED_MODE_MANUAL;
static LedState s_led_state = LED_OFF;
static uint16_t s_blink_period = 1000;    /* 慢速闪烁周期（ms） */
static uint16_t s_fast_period = 200;      /* 快速闪烁周期（ms） */
static uint32_t s_blink_tick = 0;         /* 闪烁计时 */

/* 模式名称表 */
static const char *s_mode_names[] = {
    "MANUAL",   /* 0 */
    "BLINK",    /* 1 */
    "FAST"      /* 2 */
};

/**
 * @brief  初始化 LED 模块
 */
void Module_LED_Init(void)
{
    s_mode = LED_MODE_MANUAL;
    s_led_state = LED_OFF;
    s_blink_period = 1000;
    s_fast_period = 200;
    s_blink_tick = 0;

    BSP_LED_Init();
    BSP_LED_Set(LED_OFF);
}

/**
 * @brief  设置 LED 工作模式
 */
void Module_LED_SetMode(LedMode mode)
{
    if (mode > LED_MODE_FAST_BLINK) return;
    if (mode == s_mode) return;

    /* 切换时重置计时 */
    s_blink_tick = 0;

    /* 如果从闪烁模式切到手动模式，保持当前 LED 状态 */
    /* 如果从手动模式切到闪烁模式，LED 从当前状态开始闪烁 */

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
 * @brief  获取模式名称
 */
const char* Module_LED_GetModeName(LedMode mode)
{
    if (mode > LED_MODE_FAST_BLINK) return "UNKNOWN";
    return s_mode_names[mode];
}

/**
 * @brief  设置 LED 状态
 * @note   如果当前在闪烁模式，调用此函数会切换到手动模式
 */
void Module_LED_SetState(LedState state)
{
    if (s_mode != LED_MODE_MANUAL) {
        s_mode = LED_MODE_MANUAL;
        s_blink_tick = 0;
    }
    s_led_state = state;
    BSP_LED_Set(state);
}

/**
 * @brief  翻转 LED
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
 * @brief  设置（慢速）闪烁周期
 */
void Module_LED_SetBlinkPeriod(uint16_t period_ms)
{
    if (period_ms < 100)  period_ms = 100;
    if (period_ms > 5000) period_ms = 5000;
    s_blink_period = period_ms;
}

/**
 * @brief  获取（慢速）闪烁周期
 */
uint16_t Module_LED_GetBlinkPeriod(void)
{
    return s_blink_period;
}

/**
 * @brief  设置快速闪烁周期
 */
void Module_LED_SetFastPeriod(uint16_t period_ms)
{
    if (period_ms < 100)  period_ms = 100;
    if (period_ms > 5000) period_ms = 5000;
    s_fast_period = period_ms;
}

/**
 * @brief  获取快速闪烁周期
 */
uint16_t Module_LED_GetFastPeriod(void)
{
    return s_fast_period;
}

/**
 * @brief  LED 模块周期处理
 * @note   建议 10ms 调用一次，使用 HAL_GetTick() 作为时基
 */
void Module_LED_Process(void)
{
    static uint32_t last_tick = 0;
    uint32_t now = HAL_GetTick();
    uint32_t delta;
    uint16_t period;

    if (last_tick == 0) {
        last_tick = now;
        return;
    }

    delta = now - last_tick;
    last_tick = now;

    if (s_mode == LED_MODE_BLINK || s_mode == LED_MODE_FAST_BLINK) {
        period = (s_mode == LED_MODE_BLINK) ? s_blink_period : s_fast_period;

        s_blink_tick += delta;
        if (s_blink_tick >= period) {
            s_blink_tick = 0;
            /* 翻转 LED */
            s_led_state = (s_led_state == LED_ON) ? LED_OFF : LED_ON;
            BSP_LED_Set(s_led_state);
        }
    }
}
