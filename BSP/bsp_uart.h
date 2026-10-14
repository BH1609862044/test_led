/**
 *********************************************************************
 * @file    bsp_uart.h
 * @brief   串口板级封装头文件
 * @note    只负责底层收发，不做协议解析
 *********************************************************************
 */
#ifndef __BSP_UART_H
#define __BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 串口收发接口 ---- */

/**
 * @brief  初始化串口 BSP 层（开启中断接收等）
 */
void BSP_UART_Init(void);

/**
 * @brief  串口发送字符串（阻塞方式，简短信息使用）
 * @param  str  字符串指针
 */
void BSP_UART_SendString(const char *str);

/**
 * @brief  串口发送指定长度数据（阻塞方式）
 * @param  data  数据指针
 * @param  len   数据长度
 */
void BSP_UART_SendData(const uint8_t *data, uint16_t len);

/**
 * @brief  开启串口中断接收一个字节（内部使用）
 */
void BSP_UART_StartRxIT(void);

/**
 * @brief  获取 UART 句柄指针（供 Device 层回调注册）
 * @return UART_HandleTypeDef*
 */
UART_HandleTypeDef *BSP_UART_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_UART_H */
