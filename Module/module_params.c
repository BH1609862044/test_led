/**
 *********************************************************************
 * @file    module_params.c
 * @brief   参数管理模块实现
 *********************************************************************
 */
#include "module_params.h"
#include "device_flash.h"
#include <string.h>

/* ---- 私有变量 ---- */
static ParamsStruct s_params;
static const char s_version_str[] = "PARAM v1.0";

/* ---- 默认参数 ---- */
static const ParamsStruct s_default_params = {
    .magic           = FLASH_MAGIC,
    .version         = 0x0100,
    .led_mode        = 0,       /* LED_MODE_MANUAL */
    .led_state       = 0,       /* LED_OFF */
    .blink_period_ms = PARAM_BLINK_PERIOD_DEFAULT,
    .fast_period_ms  = 200,
    .menu_start_page = 0,       /* PAGE_MAIN */
    .reserved        = 0,
    .checksum        = 0        /* 计算时再填 */
};

/* ---- 私有函数 ---- */
static uint16_t calc_checksum(const ParamsStruct *p);

/**
 * @brief  初始化
 */
void Module_Params_Init(void)
{
    ParamsStruct flash_params;

    /* 从 Flash 读取 */
    Device_Flash_Read(FLASH_PARAM_PAGE_ADDR, (uint8_t *)&flash_params, sizeof(ParamsStruct));

    /* 校验 magic + checksum */
    if (flash_params.magic == FLASH_MAGIC &&
        flash_params.version == 0x0100 &&
        flash_params.led_mode <= 2 &&
        flash_params.led_state <= 1 &&
        flash_params.blink_period_ms >= PARAM_BLINK_PERIOD_MIN &&
        flash_params.blink_period_ms <= PARAM_BLINK_PERIOD_MAX &&
        flash_params.fast_period_ms >= PARAM_BLINK_PERIOD_MIN &&
        flash_params.fast_period_ms <= PARAM_BLINK_PERIOD_MAX) {
        uint16_t calc = calc_checksum(&flash_params);
        if (calc == flash_params.checksum) {
            /* 校验通过，使用 Flash 中的参数 */
            memcpy(&s_params, &flash_params, sizeof(ParamsStruct));
            return;
        }
    }

    /* 校验失败，加载默认参数 */
    Module_Params_LoadDefault();
}

/**
 * @brief  保存到 Flash
 */
HAL_StatusTypeDef Module_Params_Save(void)
{
    /* 重新计算校验和 */
    s_params.magic = FLASH_MAGIC;
    s_params.checksum = calc_checksum(&s_params);

    return Device_Flash_WritePage(FLASH_PARAM_PAGE_ADDR,
                                   (const uint8_t *)&s_params,
                                   sizeof(ParamsStruct));
}

/**
 * @brief  恢复默认参数（不写 Flash）
 */
void Module_Params_LoadDefault(void)
{
    memcpy(&s_params, &s_default_params, sizeof(ParamsStruct));
    s_params.checksum = calc_checksum(&s_params);
}

/**
 * @brief  获取参数指针
 */
const ParamsStruct* Module_Params_Get(void)
{
    return &s_params;
}

/**
 * @brief  设置 LED 模式
 */
void Module_Params_SetLedMode(uint8_t mode)
{
    s_params.led_mode = mode;
}

/**
 * @brief  设置 LED 状态
 */
void Module_Params_SetLedState(uint8_t state)
{
    s_params.led_state = state;
}

/**
 * @brief  设置闪烁周期（带范围校验）
 */
uint8_t Module_Params_SetBlinkPeriod(uint16_t period_ms)
{
    if (period_ms < PARAM_BLINK_PERIOD_MIN || period_ms > PARAM_BLINK_PERIOD_MAX) {
        return 0;
    }
    s_params.blink_period_ms = period_ms;
    return 1;
}

/**
 * @brief  设置快速闪烁周期（带范围校验）
 */
uint8_t Module_Params_SetFastPeriod(uint16_t period_ms)
{
    if (period_ms < PARAM_BLINK_PERIOD_MIN || period_ms > PARAM_BLINK_PERIOD_MAX) {
        return 0;
    }
    s_params.fast_period_ms = period_ms;
    return 1;
}

/**
 * @brief  获取版本字符串
 */
const char* Module_Params_GetVersionStr(void)
{
    return s_version_str;
}

/* ---- 内部函数：计算 16 位校验和 ---- */
static uint16_t calc_checksum(const ParamsStruct *p)
{
    uint32_t sum = 0;
    const uint8_t *buf = (const uint8_t *)p;
    uint16_t len = sizeof(ParamsStruct) - sizeof(uint16_t); /* 去掉 checksum 字段 */
    uint16_t i;

    for (i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint16_t)(~sum + 1);  /* 取补码 */
}
