/**
 *********************************************************************
 * @file    bsp_led.h
 * @brief   板载 LED 板级封装头文件
 * @note    统一封装 LED 有效电平，业务层不直接操作 GPIO
 *          板载 LED：PC13（低电平点亮）
 *          外接 PWM LED：PA1（TIM2_CH2，PSC=71 ARR=999 -> 1kHz）
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
void     BSP_LED_Init(void);     /* 初始化板载 LED GPIO（CubeMX 已初始化则可选） */
void     BSP_LED_Set(LedState state);  /* 设置 LED 状态（ON/OFF） */
void     BSP_LED_Toggle(void);   /* 翻转 LED 状态 */
LedState BSP_LED_Get(void);      /* 读取当前 LED 状态 */

/* ---- 外接 PWM LED（PA1 / TIM2_CH2） ---- */
#define BSP_PWM_DUTY_MAX     1000U   /* 占空比量程，对应 ARR = 999 */
#define BSP_PWM_PERCENT_MAX  100U

void     BSP_PWM_Init(void);                  /* 启动 PWM 通道，占空比清零 */
void     BSP_PWM_SetPercent(uint8_t percent); /* 设置亮度 0~100% */
void     BSP_PWM_SetDuty(uint16_t duty);      /* 设置占空比 0~1000 */
uint16_t BSP_PWM_GetDuty(void);               /* 读取当前占空比 */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LED_H */
