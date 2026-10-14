/**
 *********************************************************************
 * @file    bsp_led.c
 * @brief   板载 LED 板级封装实现
 * @note    Blue Pill 板载 LED 接 PC13，低电平点亮
 *          有效电平在此层统一处理，业务层只关心 ON/OFF
 *********************************************************************
 */
#include "bsp_led.h"
#include "main.h"   // 包含 LED_GPIO_Port / LED_Pin 定义 

// LED 状态变量 
static LedState s_led_state = LED_OFF;

/**
 * @brief  初始化 LED（CubeMX 已初始化，此处仅确保初始状态）
 */
void BSP_LED_Init(void)
{
    /* 默认熄灭 */
    BSP_LED_Set(LED_OFF);
}

/**
 * @brief  设置 LED 状态
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
