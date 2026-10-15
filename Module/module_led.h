/**
 *********************************************************************
 * @file    module_led.h
 * @brief   LED 控制业务模块头文件
 * @note    业务层，管理 LED 工作模式（手动 / 闪烁）
 *          依赖 BSP LED 接口，不直接操作 GPIO
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
    LED_MODE_MANUAL = 0,   /* 手动模式 */
    LED_MODE_BLINK  = 1    /* 固定周期闪烁模式 */
} LedMode;

/* ---- 模块接口 ---- */

/**
 * @brief  初始化 LED 模块
 */
void Module_LED_Init(void);

/**
 * @brief  设置 LED 工作模式
 * @param  mode  LED_MODE_MANUAL / LED_MODE_BLINK
 */
void Module_LED_SetMode(LedMode mode);

/**
 * @brief  获取当前 LED 工作模式
 * @return 当前模式
 */
LedMode Module_LED_GetMode(void);

/**
 * @brief  手动模式下设置 LED 状态
 * @param  state  LED_ON / LED_OFF
 * @note   在闪烁模式下调用会切换到手动模式
 */
void Module_LED_SetState(LedState state);

/**
 * @brief  翻转 LED（手动模式）
 * @note   在闪烁模式下调用会切换到手动模式
 */
void Module_LED_Toggle(void);

/**
 * @brief  获取当前 LED 状态
 * @return LED_ON / LED_OFF
 */
LedState Module_LED_GetState(void);

/**
 * @brief  设置闪烁周期
 * @param  period_ms  周期（毫秒），范围 100~5000
 */
void Module_LED_SetBlinkPeriod(uint16_t period_ms);

/**
 * @brief  获取闪烁周期
 * @return 周期（毫秒）
 */
uint16_t Module_LED_GetBlinkPeriod(void);

/**
 * @brief  LED 模块周期处理（需周期调用，建议 10ms 一次）
 * @note   内部处理闪烁计时
 */
void Module_LED_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_LED_H */
