/**
 *********************************************************************
 * @file    device_uart.h
 * @brief   串口设备层头文件
 * @note    实现按行接收、环形缓冲、行就绪标志
 *          中断回调只负责接收字节和置位标志，不解析命令
 *********************************************************************
 */
#ifndef __DEVICE_UART_H
#define __DEVICE_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 配置 ---- */
#define UART_RX_BUF_SIZE   64     //单条命令最大长度 
#define UART_CMD_BUF_MAX   8      //最大命令行数（环形缓冲区大小）

/* ---- 设备层接口 ---- */

/**
 * @brief  初始化串口设备层（开启中断接收）
 */
void Device_UART_Init(void);

/**
 * @brief  发送字符串
 * @param  str  字符串
 */
void Device_UART_SendString(const char *str);

/**
 * @brief  查询是否有完整行就绪
 * @return 1 = 有完整行，0 = 没有
 */
uint8_t Device_UART_IsLineReady(void);

/**
 * @brief  读取一行命令（读完后清除就绪标志）
 * @param  buf   输出缓冲区
 * @param  size  缓冲区大小
 * @return 实际读取的字符数
 */
uint16_t Device_UART_ReadLine(char *buf, uint16_t size);

/**
 * @brief  获取串口接收字节计数
 * @return 累计接收字节数
 */
uint32_t Device_UART_GetRxCount(void);

/**
 * @brief  获取串口命令接收计数
 * @return 累计接收命令行数
 */
uint32_t Device_UART_GetCmdCount(void);

/**
 * @brief  串口中断接收回调（内部使用，在 main.c 的 HAL_UART_RxCpltCallback 中调用）
 */
void Device_UART_RxCpltCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_UART_H */
