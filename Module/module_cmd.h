/**
 *********************************************************************
 * @file    module_cmd.h
 * @brief   串口命令解析模块头文件
 * @note    Module 层，解析串口接收到的命令行并执行对应操作
 *          支持基础命令和提高题命令（SET/GET/SAVE/RESET）
 *          命令解析在任务中执行，不在中断里
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
    CMD_OK = 0,
    CMD_ERR_UNKNOWN,       /* 未知命令 */
    CMD_ERR_EMPTY,         /* 空命令 */
    CMD_ERR_BAD_STATE,     /* LED 状态参数错误 */
    CMD_ERR_MODE_RANGE,    /* 模式参数越界 */
    CMD_ERR_PERIOD_RANGE,  /* 周期参数越界 */
    CMD_ERR_TOO_LONG,      /* 命令超长 */
    CMD_ERR_SAVE_FAIL      /* 保存失败 */
} CmdResult;

/* ---- 模块接口 ---- */

/**
 * @brief  命令模块初始化
 */
void Module_Cmd_Init(void);

/**
 * @brief  命令处理（需在主循环/任务中轮询调用）
 * @note   检查串口行缓冲区是否有新的一行，有则解析并执行
 */
void Module_Cmd_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_CMD_H */
