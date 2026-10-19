/**
 *********************************************************************
 * @file    bsp_led.h
 * @brief   板载 LED 板级封装头文件
 * @note    统一封装 LED 有效电平，业务层不直接操作 GPIO
 *********************************************************************
 */
#ifndef __BSP_LED_H
#define __BSP_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* ---- LED 状态定义 ---- */
typedef enum {
    LED_OFF = 0,  /* LED 熄灭 */
    LED_ON  = 1   /* LED 点亮 */
} LedState;

/* ---- 板载 LED 操作接口 ---- */
void     BSP_LED_Init(void);     /* 初始化 LED GPIO（如需要，CubeMX 已初始化则可选） */
void     BSP_LED_Set(LedState state);  /* 设置 LED 状态（ON/OFF） */
void     BSP_LED_Toggle(void);   /* 翻转 LED 状态 */
LedState BSP_LED_Get(void);      /* 读取当前 LED 状态 */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LED_H */
