/**
 *********************************************************************
 * @file    module_led.c
 * @brief   LED 控制业务模块实现
 * @note    支持 4 种模式：手动 / 固定闪烁 / 快速闪烁 / PWM 呼吸灯
 *          板载 LED（PC13）与 PWM LED（PA1/TIM2_CH2）同步输出
 *          呼吸灯：亮度按三角波 0 -> 100 -> 0 渐变，周期可配
 *********************************************************************
 */
#include "module_led.h"

/* ---- 模块状态 ---- */
static LedMode  s_mode = LED_MODE_MANUAL;
static LedState s_led_state = LED_OFF;
static uint8_t  s_brightness = 0;              /* 当前亮度 0~100 */

static uint16_t s_blink_period = 1000;         /* 慢速闪烁周期（ms） */
static uint16_t s_fast_period = 200;           /* 快速闪烁周期（ms） */
static uint16_t s_breath_period = LED_BREATH_PERIOD_DEF;  /* 呼吸周期（ms） */

static uint32_t s_blink_tick = 0;              /* 闪烁/呼吸计时 */

/* 模式名称表（下标与 LedMode 一致） */
static const char *s_mode_names[] = {
    "MANUAL",   /* 0 */
    "BLINK",    /* 1 */
    "FAST",     /* 2 */
    "BREATH"    /* 3 */
};

#define LED_MODE_NAME_COUNT  (sizeof(s_mode_names) / sizeof(s_mode_names[0]))

/* ---- 私有函数 ---- */
static void led_apply_output(void);

/**
 * @brief  把当前状态输出到板载 LED 与 PWM LED
 */
static void led_apply_output(void)
{
    BSP_LED_Set(s_led_state);
    BSP_PWM_SetPercent(s_brightness);
}

/**
 * @brief  初始化 LED 模块
 */
void Module_LED_Init(void)
{
    s_mode = LED_MODE_MANUAL;
    s_led_state = LED_OFF;
    s_brightness = 0;
    s_blink_period = 1000;
    s_fast_period = 200;
    s_breath_period = LED_BREATH_PERIOD_DEF;
    s_blink_tick = 0;

    BSP_LED_Init();
    BSP_PWM_Init();        /* 启动 TIM2_CH2 PWM，占空比 0 */
    led_apply_output();
}

/**
 * @brief  设置 LED 工作模式
 */
void Module_LED_SetMode(LedMode mode)
{
    if (mode > LED_MODE_BREATH) return;
    if (mode == s_mode) return;

    /* 切换时重置计时 */
    s_blink_tick = 0;

    s_mode = mode;

    /* 进入呼吸模式：PWM 从最暗开始渐变，板载灯保持原状态不动 */
    if (mode == LED_MODE_BREATH) {
        s_brightness = 0;
    } else if (mode == LED_MODE_MANUAL || mode == LED_MODE_BLINK ||
               mode == LED_MODE_FAST_BLINK) {
        s_brightness = (s_led_state == LED_ON) ? 100 : 0;
    }

    led_apply_output();
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
    if ((uint8_t)mode >= LED_MODE_NAME_COUNT) return "UNKNOWN";
    return s_mode_names[mode];
}

/**
 * @brief  设置 LED 状态（手动模式）
 */
void Module_LED_SetState(LedState state)
{
    if (s_mode != LED_MODE_MANUAL) {
        s_mode = LED_MODE_MANUAL;
        s_blink_tick = 0;
    }
    s_led_state = state;
    s_brightness = (state == LED_ON) ? 100 : 0;
    led_apply_output();
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
 * @brief  单独设置板载 LED 状态（不改变模式）
 */
void Module_LED_SetMainLed(LedState state)
{
    s_led_state = state;
    BSP_LED_Set(state);
}

/**
 * @brief  获取当前亮度（0~100）
 */
uint8_t Module_LED_GetBrightness(void)
{
    return s_brightness;
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
 * @brief  设置呼吸周期（越界自动钳位，不会死机）
 */
void Module_LED_SetBreathPeriod(uint16_t period_ms)
{
    if (period_ms < LED_BREATH_PERIOD_MIN) period_ms = LED_BREATH_PERIOD_MIN;
    if (period_ms > LED_BREATH_PERIOD_MAX) period_ms = LED_BREATH_PERIOD_MAX;
    s_breath_period = period_ms;
}

/**
 * @brief  获取呼吸周期
 */
uint16_t Module_LED_GetBreathPeriod(void)
{
    return s_breath_period;
}

/**
 * @brief  按当前模块状态重新同步一次输出
 */
void Module_LED_SyncOutput(void)
{
    led_apply_output();
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
    uint8_t  brightness;

    if (last_tick == 0) {
        last_tick = now;
        return;
    }

    delta = now - last_tick;
    last_tick = now;

    switch (s_mode) {

    /* ---- 闪烁模式 ---- */
    case LED_MODE_BLINK:
    case LED_MODE_FAST_BLINK:
        period = (s_mode == LED_MODE_BLINK) ? s_blink_period : s_fast_period;
        if (period < 1) period = 1;

        s_blink_tick += delta;
        if (s_blink_tick >= period) {
            s_blink_tick %= period;
            s_led_state = (s_led_state == LED_ON) ? LED_OFF : LED_ON;
            s_brightness = (s_led_state == LED_ON) ? 100 : 0;
            led_apply_output();
        }
        break;

    /* ---- 呼吸灯模式 ---- */
    case LED_MODE_BREATH:
        {
            uint32_t half = (uint32_t)s_breath_period / 2U;
            if (half == 0) half = 1;

            s_blink_tick += delta;
            if (s_blink_tick >= s_breath_period) {
                s_blink_tick %= s_breath_period;
            }

            if (s_blink_tick < half) {
                /* 渐亮：0 -> 100 */
                brightness = (uint8_t)((s_blink_tick * 100U) / half);
            } else {
                /* 渐暗：100 -> 0 */
                brightness = (uint8_t)(100U - ((s_blink_tick - half) * 100U) / half);
            }

            if (brightness != s_brightness) {
                s_brightness = brightness;
                BSP_PWM_SetPercent(s_brightness);
            }

            /* 呼吸模式只驱动外接 PWM LED（PA1）；
             * 板载 LED（PC13）保持用户设定的 ON/OFF 不动，
             * 这样两个灯可以各自呈现不同状态 */
        }
        break;

    /* ---- 手动模式 ---- */
    default:
        break;
    }
}
