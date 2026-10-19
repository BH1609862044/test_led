/**
 *********************************************************************
 * @file    module_params.h
 * @brief   参数管理模块头文件
 * @note    Module 层，管理可配置参数和 Flash 保存/恢复
 *          参数包括：LED模式、闪烁周期、默认LED状态、配置版本
 *          Flash 数据结构带 magic + 校验，无效时恢复默认值
 *********************************************************************
 */
#ifndef __MODULE_PARAMS_H
#define __MODULE_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 参数范围定义 ---- */
#define PARAM_BLINK_PERIOD_MIN     100     /* 最小闪烁周期（ms） */
#define PARAM_BLINK_PERIOD_MAX     5000    /* 最大闪烁周期（ms） */
#define PARAM_BLINK_PERIOD_DEFAULT 1000    /* 默认闪烁周期（ms） */

/* ---- 可保存参数结构 ---- */
typedef struct {
    uint32_t  magic;           /* 有效标志：0xA5A55A5A */
    uint16_t  version;         /* 配置版本，例如 0x0100 = v1.0 */
    uint8_t   led_mode;        /* LED 工作模式（LedMode） */
    uint8_t   led_state;       /* 默认 LED 状态（LedState） */
    uint16_t  blink_period_ms; /* 闪烁周期（ms） */
    uint16_t  fast_period_ms;  /* 快速闪烁周期（ms） */
    uint8_t   menu_start_page; /* 菜单起始页 */
    uint8_t   reserved;        /* 保留 */
    uint16_t  checksum;        /* 校验和（前面所有字节的 16 位累加和取反） */
} ParamsStruct;

/* ---- 模块接口 ---- */

/**
 * @brief  参数模块初始化
 * @note   从 Flash 读取参数，校验无效则恢复默认值
 */
void Module_Params_Init(void);

/**
 * @brief  保存当前参数到 Flash
 * @return HAL_OK / HAL_ERROR
 * @note   会先擦除再写入，调用前请确认所有参数已设置好
 */
HAL_StatusTypeDef Module_Params_Save(void);

/**
 * @brief  恢复默认参数（不保存到 Flash）
 */
void Module_Params_LoadDefault(void);

/**
 * @brief  获取当前参数（只读指针）
 * @return 参数结构指针
 */
const ParamsStruct* Module_Params_Get(void);

/**
 * @brief  设置 LED 模式参数
 * @param  mode  LED 工作模式
 */
void Module_Params_SetLedMode(uint8_t mode);

/**
 * @brief  设置 LED 状态参数
 * @param  state  LED 状态
 */
void Module_Params_SetLedState(uint8_t state);

/**
 * @brief  设置闪烁周期参数
 * @param  period_ms  周期（毫秒）
 * @return 1 = 成功，0 = 参数越界
 */
uint8_t Module_Params_SetBlinkPeriod(uint16_t period_ms);

/**
 * @brief  设置快速闪烁周期参数
 * @param  period_ms  周期（毫秒）
 * @return 1 = 成功，0 = 参数越界
 */
uint8_t Module_Params_SetFastPeriod(uint16_t period_ms);

/**
 * @brief  获取配置版本字符串
 * @return 版本字符串
 */
const char* Module_Params_GetVersionStr(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_PARAMS_H */
