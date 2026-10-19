/**
 *********************************************************************
 * @file    module_sysinfo.h
 * @brief   系统状态信息模块头文件
 * @note    Module 层，收集和管理系统运行时信息
 *          包括：命令计数、错误信息、运行时间、版本号等
 *          供 OLED 系统信息页和 GET STATUS 命令使用
 *********************************************************************
 */
#ifndef __MODULE_SYSINFO_H
#define __MODULE_SYSINFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* 错误信息最大长度 */
#define SYSINFO_ERR_STR_MAX    16

/* ---- 模块接口 ---- */

/**
 * @brief  系统信息模块初始化
 */
void Module_SysInfo_Init(void);

/**
 * @brief  增加串口命令成功计数
 */
void Module_SysInfo_IncCmdOk(void);

/**
 * @brief  增加串口命令错误计数
 */
void Module_SysInfo_IncCmdErr(void);

/**
 * @brief  设置最近一次错误信息
 * @param  err_str  错误字符串
 */
void Module_SysInfo_SetLastError(const char *err_str);

/**
 * @brief  获取命令成功次数
 */
uint32_t Module_SysInfo_GetCmdOkCount(void);

/**
 * @brief  获取命令错误次数
 */
uint32_t Module_SysInfo_GetCmdErrCount(void);

/**
 * @brief  获取最近错误信息
 * @return 错误字符串指针
 */
const char* Module_SysInfo_GetLastError(void);

/**
 * @brief  获取软件版本字符串
 * @return 版本号字符串
 */
const char* Module_SysInfo_GetSwVersion(void);

/**
 * @brief  获取系统运行时间（秒）
 * @return 运行秒数
 */
uint32_t Module_SysInfo_GetUptime(void);

/**
 * @brief  更新运行时间（需每秒调用一次）
 */
void Module_SysInfo_TickSecond(void);

/**
 * @brief  清除所有统计信息
 */
void Module_SysInfo_ResetStats(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_SYSINFO_H */
