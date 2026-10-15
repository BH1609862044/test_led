/**
 *********************************************************************
 * @file    module_cmd.h
 * @brief   串口命令解析模块头文件
 * @note    基础题命令：HELP, LED ON/OFF/TOGGLE, STATUS
 *          在主循环中调用 Cmd_Process() 处理就绪的命令行
 *          命令解析结果通过 Module_LED 等模块生效
 *********************************************************************
 */
#ifndef __MODULE_CMD_H
#define __MODULE_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 错误码 ---- */
typedef enum {
    CMD_OK = 0,              /* 成功 */
    CMD_ERR_UNKNOWN = 1,     /* 未知命令 */
    CMD_ERR_BAD_LED = 2,     /* LED 参数错误 */
    CMD_ERR_EMPTY = 3,       /* 空命令 */
    CMD_ERR_TOO_LONG = 4     /* 命令超长 */
} CmdResult;

/* ---- 模块接口 ---- */

/**
 * @brief  命令模块初始化
 */
void Module_Cmd_Init(void);

/**
 * @brief  命令处理（在主循环中调用）
 * @note   检查 Device_UART 是否有就绪行，有则解析并执行
 */
void Module_Cmd_Process(void);

/**
 * @brief  获取成功命令计数
 * @return 成功执行的命令数
 */
uint32_t Module_Cmd_GetSuccessCount(void);

/**
 * @brief  获取错误命令计数
 * @return 错误命令数
 */
uint32_t Module_Cmd_GetErrorCount(void);

/**
 * @brief  获取最近一次错误码
 * @return 最近错误码
 */
CmdResult Module_Cmd_GetLastError(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_CMD_H */
