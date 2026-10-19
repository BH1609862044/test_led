/**
 *********************************************************************
 * @file    app.h
 * @brief   应用层头文件
 * @note    应用入口和全局状态管理
 *          负责按键事件回调、系统初始化编排、启动自检流程
 *********************************************************************
 */
#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "bsp_key.h"
/* ---- 软件版本 ---- */
#define APP_VERSION_STR    "v1.0.0"
#define APP_PROJECT_NAME   "STM32 OLED LED Control Console"

/* ---- 应用层接口 ---- */

/**
 * @brief  应用初始化（在 main.c 的 MX 初始化之后调用）
 * @note   依次初始化 BSP → Device → Component → Module → Task
 */
void App_Init(void);

/**
 * @brief  应用主循环（在 main.c 的 while(1) 中调用）
 */
void App_Loop(void);

/**
 * @brief  按键事件回调（由 Component/Key 调用）
 * @param  key_id  按键编号
 * @param  event   事件类型
 */
void App_OnKeyEvent(KeyID key_id, KeyLevel event);

/**
 * @brief  获取系统运行时间（秒）
 * @return 运行秒数
 */
uint32_t App_GetUptimeSeconds(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */
