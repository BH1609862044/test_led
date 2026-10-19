/**
 *********************************************************************
 * @file    module_sysinfo.c
 * @brief   系统状态信息模块实现
 *********************************************************************
 */
#include "module_sysinfo.h"
#include <string.h>

/* ---- 软件版本 ---- */
static const char s_sw_version[] = "v1.1.0";

/* ---- 统计数据 ---- */
static uint32_t s_cmd_ok_count = 0;
static uint32_t s_cmd_err_count = 0;
static uint32_t s_uptime_seconds = 0;
static char s_last_error[SYSINFO_ERR_STR_MAX] = "NONE";

/**
 * @brief  初始化
 */
void Module_SysInfo_Init(void)
{
    s_cmd_ok_count = 0;
    s_cmd_err_count = 0;
    s_uptime_seconds = 0;
    strncpy(s_last_error, "NONE", SYSINFO_ERR_STR_MAX - 1);
    s_last_error[SYSINFO_ERR_STR_MAX - 1] = '\0';
}

/**
 * @brief  命令成功计数 +1
 */
void Module_SysInfo_IncCmdOk(void)
{
    s_cmd_ok_count++;
}

/**
 * @brief  命令错误计数 +1
 */
void Module_SysInfo_IncCmdErr(void)
{
    s_cmd_err_count++;
}

/**
 * @brief  设置最近错误信息
 */
void Module_SysInfo_SetLastError(const char *err_str)
{
    if (err_str == NULL) return;
    strncpy(s_last_error, err_str, SYSINFO_ERR_STR_MAX - 1);
    s_last_error[SYSINFO_ERR_STR_MAX - 1] = '\0';
}

/**
 * @brief  获取成功次数
 */
uint32_t Module_SysInfo_GetCmdOkCount(void)
{
    return s_cmd_ok_count;
}

/**
 * @brief  获取错误次数
 */
uint32_t Module_SysInfo_GetCmdErrCount(void)
{
    return s_cmd_err_count;
}

/**
 * @brief  获取最近错误
 */
const char* Module_SysInfo_GetLastError(void)
{
    return s_last_error;
}

/**
 * @brief  获取软件版本
 */
const char* Module_SysInfo_GetSwVersion(void)
{
    return s_sw_version;
}

/**
 * @brief  获取运行时间（秒）
 */
uint32_t Module_SysInfo_GetUptime(void)
{
    return s_uptime_seconds;
}

/**
 * @brief  秒级递增
 */
void Module_SysInfo_TickSecond(void)
{
    s_uptime_seconds++;
}

/**
 * @brief  重置统计
 */
void Module_SysInfo_ResetStats(void)
{
    s_cmd_ok_count = 0;
    s_cmd_err_count = 0;
    s_uptime_seconds = 0;
    strncpy(s_last_error, "NONE", SYSINFO_ERR_STR_MAX - 1);
    s_last_error[SYSINFO_ERR_STR_MAX - 1] = '\0';
}
