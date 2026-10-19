/**
 *********************************************************************
 * @file    device_uart.c
 * @brief   串口设备层实现
 * @note    按行接收：遇到 \n 认为一行结束，去除 \r
 *          中断中只负责收字节和置标志，不做命令解析
 *          单条命令最长 UART_RX_BUF_SIZE，超长丢弃本行
 *********************************************************************
 */
#include "device_uart.h"
#include "bsp_uart.h"
#include <string.h>

/* ---- 接收缓冲区 ---- */
static uint8_t  s_rx_byte;                       // 单字节接收缓冲 
static char     s_rx_buf[UART_RX_BUF_SIZE];      // 当前行缓冲区 
static uint16_t s_rx_idx = 0;                    // 当前行写入位置 
static uint8_t  s_line_overflow = 0;             // 当前行超长标志

typedef struct {
    char data[UART_RX_BUF_SIZE];
    uint16_t len;
    uint8_t too_long;
} UARTLine;

static UARTLine s_line_queue[UART_CMD_BUF_MAX + 1];
static volatile uint8_t s_queue_head = 0;
static volatile uint8_t s_queue_tail = 0;
static volatile uint8_t s_queue_overflow = 0;

/* ---- 统计计数 ---- */
static uint32_t s_rx_byte_count = 0;  // 累计接收字节数
static uint32_t s_cmd_count = 0;      // 累计命令行数

/**
 * @brief  初始化串口设备层
 */
void Device_UART_Init(void)
{
    s_rx_idx = 0;
    s_line_overflow = 0;
    s_queue_head = 0;
    s_queue_tail = 0;
    s_queue_overflow = 0;
    s_rx_byte_count = 0;
    s_cmd_count = 0;

    /* 开启第一个字节的中断接收 */
    HAL_UART_Receive_IT(BSP_UART_GetHandle(), &s_rx_byte, 1);
}

/**
 * @brief  发送字符串（委托给 BSP 层）
 */
void Device_UART_SendString(const char *str)
{
    BSP_UART_SendString(str);
}

/**
 * @brief  查询是否有完整行就绪
 */
uint8_t Device_UART_IsLineReady(void)
{
    return (s_queue_head != s_queue_tail) || s_queue_overflow;
}

/**
 * @brief  读取一行命令
 * @param  buf   输出缓冲区
 * @param  size  缓冲区大小
 * @return 实际读取的字符数（不含 '\0'）
 */
uint16_t Device_UART_ReadLine(char *buf, uint16_t size)
{
    UARTLine *line;
    uint16_t len = 0;
    uint8_t tail;

    if (buf == 0 || size == 0) {
        return 0;
    }

    tail = s_queue_tail;
    if (tail == s_queue_head) {
        uint32_t primask = __get_PRIMASK();
        uint8_t queue_overflow;

        __disable_irq();
        queue_overflow = (s_queue_tail == s_queue_head) && s_queue_overflow;
        if (queue_overflow) {
            s_queue_overflow = 0;
        }
        __set_PRIMASK(primask);

        buf[0] = '\0';
        return queue_overflow ? UART_RX_QUEUE_FULL : 0;
    }

    line = &s_line_queue[tail];
    if (line->too_long) {
        buf[0] = '\0';
        len = UART_RX_LINE_TOO_LONG;
    } else {
        len = line->len;
        if (len >= size) {
            len = size - 1;
        }
        memcpy(buf, line->data, len);
        buf[len] = '\0';
    }

    s_queue_tail = (tail + 1) % (UART_CMD_BUF_MAX + 1);

    return len;
}

/**
 * @brief  获取累计接收字节数
 */
uint32_t Device_UART_GetRxCount(void)
{
    return s_rx_byte_count;
}

/**
 * @brief  获取累计命令行数
 */
uint32_t Device_UART_GetCmdCount(void)
{
    return s_cmd_count;
}

/**
 * @brief  串口接收完成回调（每次收到 1 字节调用一次）
 * @note   在 main.c 的 HAL_UART_RxCpltCallback 中调用此函数
 *         只做收字节、写缓冲、置标志，不做协议解析
 */
void Device_UART_RxCpltCallback(void)
{
    uint8_t next_head;

    s_rx_byte_count++;

    if (s_rx_byte == '\n') {
        s_rx_buf[s_rx_idx] = '\0';
        next_head = (s_queue_head + 1) % (UART_CMD_BUF_MAX + 1);
        if (next_head != s_queue_tail) {
            memcpy(s_line_queue[s_queue_head].data, s_rx_buf, sizeof(s_rx_buf));
            s_line_queue[s_queue_head].len = s_rx_idx;
            s_line_queue[s_queue_head].too_long = s_line_overflow;
            s_queue_head = next_head;
        } else {
            s_queue_overflow = 1;
        }
        s_cmd_count++;
        s_rx_idx = 0;
        s_line_overflow = 0;
    } else if (s_rx_byte == '\r') {
        //忽略回车符 
    } else if (s_line_overflow) {
        /* 超长行继续丢弃，直到换行 */
    } else {
        //普通字符 
        if (s_rx_idx < UART_RX_BUF_SIZE - 1) {
            s_rx_buf[s_rx_idx++] = (char)s_rx_byte;
        } else {
            //超长：标记并丢弃本行 
            s_line_overflow = 1;
        }
    }

    /* 重新开启下一字节接收 */
    HAL_UART_Receive_IT(BSP_UART_GetHandle(), &s_rx_byte, 1);
}
