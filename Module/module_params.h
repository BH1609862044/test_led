/**
 *********************************************************************
 * @file    module_params.h
 * @brief   参数管理模块头文件
 * @note    Module 层，管理可配置参数和 Flash 保存/恢复
 *          参数包括：LED 模式、慢闪/快闪/呼吸周期、默认 LED 状态、配置版本
 *          Flash 数据结构带 magic + 版本 + 校验和，任一不符则恢复默认值
 *********************************************************************
 */
#ifndef __MODULE_PARAMS_H
#define __MODULE_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "module_led.h"

/* ---- 参数版本 ---- */
#define PARAM_VERSION_CURRENT      0x0102      /* v1.2 */

/* ---- 参数范围定义 ---- */
#define PARAM_BLINK_PERIOD_MIN     100     /* 最小闪烁周期（ms） */
#define PARAM_BLINK_PERIOD_MAX     5000    /* 最大闪烁周期（ms） */
#define PARAM_BLINK_PERIOD_DEFAULT 1000    /* 默认闪烁周期（ms） */

#define PARAM_FAST_PERIOD_MIN      100
#define PARAM_FAST_PERIOD_MAX      5000
#define PARAM_FAST_PERIOD_DEFAULT  200

#define PARAM_BREATH_PERIOD_MIN    1000    /* 最小呼吸周期（ms） */
#define PARAM_BREATH_PERIOD_MAX    10000   /* 最大呼吸周期（ms） */
#define PARAM_BREATH_PERIOD_DEFAULT 4000   /* 默认呼吸周期（ms） */

#define PARAM_LED_MODE_MAX         3       /* 合法模式 0~3 */

/* ---- 可保存参数结构（20 字节，4 字节对齐，无填充） ---- */
typedef struct {
    uint32_t  magic;            /* 有效标志：0xA5A55A5A */
    uint16_t  version;          /* 配置版本 0x0102 = v1.2 */
    uint8_t   led_mode;         /* LED 工作模式（LedMode 0~3） */
    uint8_t   led_state;        /* 默认 LED 状态（LedState 0/1） */
    uint16_t  blink_period_ms;  /* 慢速闪烁周期（ms） */
    uint16_t  fast_period_ms;   /* 快速闪烁周期（ms） */
    uint16_t  breath_period_ms; /* 呼吸灯周期（ms） */
    uint8_t   menu_start_page;  /* 菜单起始页（保留） */
    uint8_t   reserved;         /* 保留 */
    uint16_t  checksum;         /* 校验和（覆盖本字段之前的全部字节） */
    uint16_t  reserved2;        /* 填充，保证结构体长度为 4 的整数倍 */
} ParamsStruct;

/* ---- 模块接口 ---- */

/**
 * @brief  参数模块初始化
 * @note   从 Flash 读取参数；magic/版本/范围/校验和任一不合法则恢复默认值
 */
void Module_Params_Init(void);

/**
 * @brief  保存当前参数到 Flash
 * @return HAL_OK / HAL_ERROR
 */
HAL_StatusTypeDef Module_Params_Save(void);

/**
 * @brief  恢复默认参数（不写 Flash）
 */
void Module_Params_LoadDefault(void);

/**
 * @brief  获取当前参数（只读指针）
 */
const ParamsStruct* Module_Params_Get(void);

/**
 * @brief  设置 LED 模式参数（越界忽略）
 */
void Module_Params_SetLedMode(uint8_t mode);

/**
 * @brief  设置 LED 状态参数（非 0 即 1）
 */
void Module_Params_SetLedState(uint8_t state);

/**
 * @brief  设置闪烁周期参数
 * @return 1 = 成功，0 = 参数越界
 */
uint8_t Module_Params_SetBlinkPeriod(uint16_t period_ms);

/**
 * @brief  设置快速闪烁周期参数
 * @return 1 = 成功，0 = 参数越界
 */
uint8_t Module_Params_SetFastPeriod(uint16_t period_ms);

/**
 * @brief  设置呼吸灯周期参数
 * @return 1 = 成功，0 = 参数越界
 */
uint8_t Module_Params_SetBreathPeriod(uint16_t period_ms);

/**
 * @brief  获取配置版本字符串
 */
const char* Module_Params_GetVersionStr(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_PARAMS_H */
