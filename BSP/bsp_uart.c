/**
 *********************************************************************
 * @file    bsp_uart.c
 * @brief   串口板级封装实现
 * @note    使用 USART1，115200-8-N-1
 *          CubeMX 已配置 huart1，此处封装收发接口
 *********************************************************************
 */
#include "bsp_uart.h"
#include "main.h"   /* 包含 huart1 声明 */
#include <string.h>

/* 外部声明：CubeMX 生成的 UART 句柄 */
extern UART_HandleTypeDef huart1;

/**
 * @brief  串口 BSP 初始化
 * @note   CubeMX 已初始化 huart1，此处开启中断接收
 */
void BSP_UART_Init(void)
{
    /* 开启接收中断 —— 具体的字节接收回调在 Device/UART 层处理 */
    BSP_UART_StartRxIT();
}

/**
 * @brief  发送字符串（阻塞式）
 * @param  str  以 '\0' 结尾的字符串
 */
void BSP_UART_SendString(const char *str)
{
    if (str == 0) return;
    uint16_t len = (uint16_t)strlen(str);
    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t *)str, len, 1000);
    }
}

/**
 * @brief  发送指定长度数据（阻塞式）
 */
void BSP_UART_SendData(const uint8_t *data, uint16_t len)
{
    if (data == 0 || len == 0) return;
    HAL_UART_Transmit(&huart1, data, len, 1000);
}

/**
 * @brief  开启一个字节的中断接收
 * @note   每次接收到 1 字节后需要重新调用
 */
void BSP_UART_StartRxIT(void)
{
    /* 接收单字节的缓冲区由 Device 层提供，这里不直接调用
     * 具体的 HAL_UART_Receive_IT 在 device_uart.c 中启动
     * 此处保留接口以维持分层清晰 */
}

/**
 * @brief  获取 UART 句柄
 */
UART_HandleTypeDef *BSP_UART_GetHandle(void)
{
    return &huart1;
}
