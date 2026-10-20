/**
 *********************************************************************
 * @file    bsp_led.c
 * @brief   板载 LED 与 PWM LED 板级封装实现
 * @note    板载 LED 接 PC13，低电平点亮
 *          外接 PWM LED 接 PA1（TIM2_CH2），1kHz PWM 调亮度
 *          有效电平和占空比换算都在此层统一处理
 *********************************************************************
 */
#include "bsp_led.h"
#include "main.h"   // 包含 LED_GPIO_Port / LED_Pin 定义
#include "tim.h"    // 包含 htim2 定义

/* 板载 LED 状态变量（仅用于 Get/Toggle，硬件才是最终真相） */
static LedState s_led_state = LED_OFF;

/* PWM 当前占空比 */
static uint16_t s_pwm_duty = 0;

/**
 * @brief  初始化板载 LED（CubeMX 已初始化，此处仅确保初始状态）
 */
void BSP_LED_Init(void)
{
    /* 默认熄灭 */
    BSP_LED_Set(LED_OFF);
}

/**
 * @brief  设置板载 LED 状态
 * @param  state  LED_ON / LED_OFF
 * @note   PC13 低电平点亮，在此统一转换有效电平
 */
void BSP_LED_Set(LedState state)
{
    if (state == LED_ON) {
        //低电平点亮
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    } else {
        //高电平熄灭
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    }
    s_led_state = state;
}

/**
 * @brief  翻转 LED 状态
 */
void BSP_LED_Toggle(void)
{
    if (s_led_state == LED_ON) {
        BSP_LED_Set(LED_OFF);
    } else {
        BSP_LED_Set(LED_ON);
    }
}

/**
 * @brief  读取当前 LED 功能状态
 * @return LED_ON / LED_OFF
 */
LedState BSP_LED_Get(void)
{
    return s_led_state;
}

/**
 * @brief  初始化 PWM 通道（PA1 / TIM2_CH2）
 * @note   TIM2 由 CubeMX 的 MX_TIM2_Init() 配置好：
 *         PSC=71 -> 1MHz 计数，ARR=999 -> 1kHz PWM，PWM1 模式，高电平有效
 */
void BSP_PWM_Init(void)
{
    s_pwm_duty = 0;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

/**
 * @brief  设置 PWM 占空比（原始值）
 * @param  duty  0~BSP_PWM_DUTY_MAX
 */
void BSP_PWM_SetDuty(uint16_t duty)
{
    if (duty > BSP_PWM_DUTY_MAX) {
        duty = BSP_PWM_DUTY_MAX;
    }
    s_pwm_duty = duty;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

/**
 * @brief  设置亮度（百分比）
 * @param  percent  0~100（>100 按 100 处理）
 */
void BSP_PWM_SetPercent(uint8_t percent)
{
    uint32_t duty;

    if (percent > BSP_PWM_PERCENT_MAX) {
        percent = BSP_PWM_PERCENT_MAX;
    }

    duty = ((uint32_t)percent * BSP_PWM_DUTY_MAX) / BSP_PWM_PERCENT_MAX;
    BSP_PWM_SetDuty((uint16_t)duty);
}

/**
 * @brief  读取当前占空比
 * @return 0~BSP_PWM_DUTY_MAX
 */
uint16_t BSP_PWM_GetDuty(void)
{
    return s_pwm_duty;
}
