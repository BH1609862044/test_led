/**
 *********************************************************************
 * @file    module_params.c
 * @brief   参数管理模块实现
 * @note    保存/恢复流程：
 *          上电 -> 读 Flash 参数页 -> 校验 magic/版本/取值范围/校验和
 *               -> 任一不通过则用默认值（不会死机，也不会越界访问）
 *          保存 -> 填 magic + 重算校验和 -> 擦页 -> 按字写入
 *********************************************************************
 */
#include "module_params.h"
#include "device_flash.h"
#include <stddef.h>
#include <string.h>

/* ---- 私有变量 ---- */
static ParamsStruct s_params;
static const char s_version_str[] = "PARAM v1.2";

/* ---- 默认参数 ---- */
static const ParamsStruct s_default_params = {
    .magic            = FLASH_MAGIC,
    .version          = PARAM_VERSION_CURRENT,
    .led_mode         = (uint8_t)LED_MODE_MANUAL,
    .led_state        = (uint8_t)LED_OFF,
    .blink_period_ms  = PARAM_BLINK_PERIOD_DEFAULT,
    .fast_period_ms   = PARAM_FAST_PERIOD_DEFAULT,
    .breath_period_ms = PARAM_BREATH_PERIOD_DEFAULT,
    .menu_start_page  = 0,
    .reserved         = 0,
    .checksum         = 0,
    .reserved2        = 0
};

/* ---- 私有函数 ---- */
static uint16_t calc_checksum(const ParamsStruct *p);
static void     clamp_params(ParamsStruct *p);
static uint8_t  params_is_valid(const ParamsStruct *p);

/**
 * @brief  初始化：从 Flash 加载，失败则恢复默认
 */
void Module_Params_Init(void)
{
    ParamsStruct flash_params;

    Device_Flash_Read(FLASH_PARAM_PAGE_ADDR, (uint8_t *)&flash_params, sizeof(ParamsStruct));

    if (params_is_valid(&flash_params)) {
        memcpy(&s_params, &flash_params, sizeof(ParamsStruct));
        clamp_params(&s_params);
        s_params.checksum = calc_checksum(&s_params);
        return;
    }

    /* 首次上电 / 参数损坏 / 版本不匹配 / 参数越界 -> 默认值 */
    Module_Params_LoadDefault();
}

/**
 * @brief  保存到 Flash
 */
HAL_StatusTypeDef Module_Params_Save(void)
{
    clamp_params(&s_params);

    s_params.magic = FLASH_MAGIC;
    s_params.version = PARAM_VERSION_CURRENT;
    s_params.checksum = calc_checksum(&s_params);

    return Device_Flash_WritePage(FLASH_PARAM_PAGE_ADDR,
                                  (const uint8_t *)&s_params,
                                  (uint16_t)sizeof(ParamsStruct));
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
 * @brief  设置 LED 模式（越界忽略）
 */
void Module_Params_SetLedMode(uint8_t mode)
{
    if (mode > PARAM_LED_MODE_MAX) return;
    s_params.led_mode = mode;
}

/**
 * @brief  设置 LED 状态
 */
void Module_Params_SetLedState(uint8_t state)
{
    s_params.led_state = (state != 0) ? 1 : 0;
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
    if (period_ms < PARAM_FAST_PERIOD_MIN || period_ms > PARAM_FAST_PERIOD_MAX) {
        return 0;
    }
    s_params.fast_period_ms = period_ms;
    return 1;
}

/**
 * @brief  设置呼吸周期（带范围校验）
 */
uint8_t Module_Params_SetBreathPeriod(uint16_t period_ms)
{
    if (period_ms < PARAM_BREATH_PERIOD_MIN || period_ms > PARAM_BREATH_PERIOD_MAX) {
        return 0;
    }
    s_params.breath_period_ms = period_ms;
    return 1;
}

/**
 * @brief  获取版本字符串
 */
const char* Module_Params_GetVersionStr(void)
{
    return s_version_str;
}

/* ============================================================
 * 内部函数
 * ============================================================ */

/**
 * @brief  参数合法性检查（不修改传入数据）
 * @return 1 = 合法，0 = 非法
 */
static uint8_t params_is_valid(const ParamsStruct *p)
{
    if (p->magic != FLASH_MAGIC) return 0;
    if (p->version != PARAM_VERSION_CURRENT) return 0;
    if (p->led_mode > PARAM_LED_MODE_MAX) return 0;
    if (p->led_state > 1) return 0;
    if (p->blink_period_ms < PARAM_BLINK_PERIOD_MIN ||
        p->blink_period_ms > PARAM_BLINK_PERIOD_MAX) return 0;
    if (p->fast_period_ms < PARAM_FAST_PERIOD_MIN ||
        p->fast_period_ms > PARAM_FAST_PERIOD_MAX) return 0;
    if (p->breath_period_ms < PARAM_BREATH_PERIOD_MIN ||
        p->breath_period_ms > PARAM_BREATH_PERIOD_MAX) return 0;
    if (calc_checksum(p) != p->checksum) return 0;
    return 1;
}

/**
 * @brief  参数钳位（兜底，保证任何情况下都不会传出非法值）
 */
static void clamp_params(ParamsStruct *p)
{
    if (p->led_mode > PARAM_LED_MODE_MAX) {
        p->led_mode = (uint8_t)LED_MODE_MANUAL;
    }
    if (p->led_state > 1) {
        p->led_state = (uint8_t)LED_OFF;
    }
    if (p->blink_period_ms < PARAM_BLINK_PERIOD_MIN ||
        p->blink_period_ms > PARAM_BLINK_PERIOD_MAX) {
        p->blink_period_ms = PARAM_BLINK_PERIOD_DEFAULT;
    }
    if (p->fast_period_ms < PARAM_FAST_PERIOD_MIN ||
        p->fast_period_ms > PARAM_FAST_PERIOD_MAX) {
        p->fast_period_ms = PARAM_FAST_PERIOD_DEFAULT;
    }
    if (p->breath_period_ms < PARAM_BREATH_PERIOD_MIN ||
        p->breath_period_ms > PARAM_BREATH_PERIOD_MAX) {
        p->breath_period_ms = PARAM_BREATH_PERIOD_DEFAULT;
    }
}

/**
 * @brief  计算 16 位校验和（覆盖 checksum 字段之前的全部字节）
 */
static uint16_t calc_checksum(const ParamsStruct *p)
{
    uint32_t sum = 0;
    const uint8_t *buf = (const uint8_t *)p;
    uint16_t len = (uint16_t)offsetof(ParamsStruct, checksum);
    uint16_t i;

    for (i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint16_t)(~sum + 1);  /* 取补码 */
}
