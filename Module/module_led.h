/**
 *********************************************************************
 * @file    module_led.h
 * @brief   LED 控制业务模块头文件
 * @note    业务层，管理 LED 工作模式
 *          支持 4 种模式：手动 / 固定闪烁 / 快速闪烁 / PWM 呼吸灯
 *          板载 LED：PC13（低电平点亮）
 *          外接 PWM LED：PA1（TIM2_CH2，1kHz）
 *********************************************************************
 */
#ifndef __MODULE_LED_H
#define __MODULE_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "bsp_led.h"

/* ---- LED 工作模式 ---- */
typedef enum {
    LED_MODE_MANUAL     = 0,   /* 手动模式 */
    LED_MODE_BLINK      = 1,   /* 固定周期闪烁模式（默认 1000ms） */
    LED_MODE_FAST_BLINK = 2,   /* 快速闪烁模式（默认 200ms） */
    LED_MODE_BREATH     = 3    /* PWM 呼吸灯模式（默认 4000ms 一个周期） */
} LedMode;

/* ---- 呼吸灯周期范围 ---- */
#define LED_BREATH_PERIOD_MIN   1000    /* 最小呼吸周期（ms） */
#define LED_BREATH_PERIOD_MAX   10000   /* 最大呼吸周期（ms） */
#define LED_BREATH_PERIOD_DEF   4000    /* 默认呼吸周期（ms） */

/* ---- 模块接口 ---- */

/**
 * @brief  初始化 LED 模块（含 PWM 通道）
 */
void Module_LED_Init(void);

/**
 * @brief  设置 LED 工作模式
 * @param  mode  LED_MODE_MANUAL / BLINK / FAST_BLINK / BREATH
 */
void Module_LED_SetMode(LedMode mode);

/**
 * @brief  获取当前 LED 工作模式
 */
LedMode Module_LED_GetMode(void);

/**
 * @brief  获取模式名称字符串
 * @param  mode  模式
 * @return 模式名称（"MANUAL"/"BLINK"/"FAST"/"BREATH"/"UNKNOWN"）
 */
const char* Module_LED_GetModeName(LedMode mode);

/**
 * @brief  手动模式下设置 LED 状态
 * @param  state  LED_ON / LED_OFF
 * @note   在非手动模式下调用会切换到手动模式
 */
void Module_LED_SetState(LedState state);

/**
 * @brief  翻转 LED（手动模式）
 */
void Module_LED_Toggle(void);

/**
 * @brief  获取当前 LED 状态
 */
LedState Module_LED_GetState(void);

/**
 * @brief  单独设置板载 LED（PC13）状态，不改变当前模式
 * @note   呼吸/闪烁模式下用它可以让板载灯与 PWM 灯各自呈现
 */
void Module_LED_SetMainLed(LedState state);

/**
 * @brief  获取当前亮度（0~100%）
 * @note   呼吸模式下返回实时亮度；手动/闪烁模式返回 0 或 100
 */
uint8_t Module_LED_GetBrightness(void);

/**
 * @brief  设置（慢速）闪烁周期
 * @param  period_ms  周期（毫秒），范围 100~5000
 */
void Module_LED_SetBlinkPeriod(uint16_t period_ms);

/**
 * @brief  获取（慢速）闪烁周期
 */
uint16_t Module_LED_GetBlinkPeriod(void);

/**
 * @brief  设置快速闪烁周期
 * @param  period_ms  周期（毫秒），范围 100~5000
 */
void Module_LED_SetFastPeriod(uint16_t period_ms);

/**
 * @brief  获取快速闪烁周期
 */
uint16_t Module_LED_GetFastPeriod(void);

/**
 * @brief  设置呼吸周期
 * @param  period_ms  完整呼吸周期（毫秒），范围 1000~10000
 */
void Module_LED_SetBreathPeriod(uint16_t period_ms);

/**
 * @brief  获取呼吸周期
 */
uint16_t Module_LED_GetBreathPeriod(void);

/**
 * @brief  LED 模块周期处理（需周期调用，建议 10ms 一次）
 * @note   内部处理闪烁计时与呼吸灯亮度渐变
 */
void Module_LED_Process(void);

/**
 * @brief  按当前模块状态重新同步一次 LED 输出
 * @note   上层直接操作过 BSP 后（如上电自检）调用
 */
void Module_LED_SyncOutput(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_LED_H */
