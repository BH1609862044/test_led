/**
 *********************************************************************
 * @file    module_display.h
 * @brief   OLED 显示业务模块头文件
 * @note    负责 OLED 界面的组织和刷新
 *          基础题实现主界面：显示模式、LED 状态、运行计数
 *          显示数据来自 Module 层，不直接操作硬件
 *********************************************************************
 */
#ifndef __MODULE_DISPLAY_H
#define __MODULE_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 页面类型 ---- */
typedef enum {
    PAGE_BOOT = 0,      /* 启动页 */
    PAGE_MAIN = 1,      /* 主界面 */
    PAGE_MAX
} PageType;

/* ---- 模块接口 ---- */

/**
 * @brief  显示模块初始化
 */
void Module_Display_Init(void);

/**
 * @brief  显示启动页（上电自检）
 */
void Module_Display_ShowBootPage(void);

/**
 * @brief  切换到主界面
 */
void Module_Display_ShowMainPage(void);

/**
 * @brief  刷新主界面（LED 状态、模式、计数等变化时调用）
 * @note   只刷新变化的行，减少闪烁
 */
void Module_Display_RefreshMain(void);

/**
 * @brief  显示模块周期刷新（建议 100ms 调用一次）
 * @note   内部刷新运行时间等动态信息
 */
void Module_Display_Process(void);

/**
 * @brief  设置错误信息（临时显示）
 * @param  err_str  错误字符串
 */
void Module_Display_ShowError(const char *err_str);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_DISPLAY_H */
