/**
 *********************************************************************
 * @file    device_uart.h
 * @brief   串口设备层头文件
 * @note    实现按行接收、环形行队列、行就绪标志
 *          中断只负责收字节、拼行、入队；命令解析在主循环里做
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
#define UART_CMD_BUF_MAX   8      //最大命令行数（环形队列深度）
#define UART_RX_LINE_TOO_LONG 0xFFFFU
#define UART_RX_QUEUE_FULL 0xFFFEU

/* ---- 设备层接口 ---- */

/**
 * @brief  初始化串口设备层（清空队列并开启中断接收）
 */
void Device_UART_Init(void);

/**
 * @brief  发送字符串
 * @param  str  字符串
 */
void Device_UART_SendString(const char *str);

/**
 * @brief  查询是否有完整行就绪（队列最多缓存 UART_CMD_BUF_MAX 条）
 * @return 1 = 有完整行，0 = 没有
 */
uint8_t Device_UART_IsLineReady(void);

/**
 * @brief  读取一行命令（读取队列最早的一条）
 * @param  buf   输出缓冲区
 * @param  size  缓冲区大小
 * @return 实际读取的字符数；超长行/队列溢出分别返回对应错误标记
 */
uint16_t Device_UART_ReadLine(char *buf, uint16_t size);

/**
 * @brief  获取串口接收字节计数
 */
uint32_t Device_UART_GetRxCount(void);

/**
 * @brief  获取串口命令接收计数
 */
uint32_t Device_UART_GetCmdCount(void);

/**
 * @brief  串口接收完成回调（在 main.c 的 HAL_UART_RxCpltCallback 中调用）
 */
void Device_UART_RxCpltCallback(void);

/**
 * @brief  串口出错后重启接收
 * @note   在 HAL_UART_ErrorCallback 中调用。
 *         HAL 检测到溢出(ORE)后会主动关闭接收中断，若不重启则串口永久失联
 */
void Device_UART_RestartRx(void);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_UART_H */
