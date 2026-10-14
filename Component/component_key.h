/**
 *********************************************************************
 * @file    component_key.h
 * @brief   按键消抖组件头文件
 * @note    通用按键消抖组件，与具体硬件无关
 *          周期调用 Key_Scan()，一次按下只产生一次短按事件
 *          按键事件通过回调函数向上层（Module）传递
 *********************************************************************
 */
#ifndef __COMPONENT_KEY_H
#define __COMPONENT_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 按键编号 ---- */
#define KEY_ID_1    0
#define KEY_ID_2    1
#define KEY_ID_3    2
#define KEY_NUM     3

/* ---- 按键事件类型 ---- */
typedef enum {
    KEY_EVT_NONE = 0,   // 无事件
    KEY_EVT_SHORT = 1   // 短按事件
} KeyEventType;

/* ---- 按键事件回调函数类型 ---- */
typedef void (*KeyEventCallback)(uint8_t key_id, KeyEventType event);

/* ---- 组件接口 ---- */

/**
 * @brief  按键组件初始化
 * @param  cb  按键事件回调函数（可为 NULL）
 */
void Key_Init(KeyEventCallback cb);

/**
 * @brief  按键扫描（需周期调用，建议 10ms 调用一次）
 * @note   内部完成消抖和事件生成
 */
void Key_Scan(void);

/**
 * @brief  查询某个按键是否有短按事件（轮询方式）
 * @param  key_id  按键编号
 * @return 1 = 有短按事件（读取后清除），0 = 无
 * @note   与回调方式二选一，或同时使用
 */
uint8_t Key_GetShortPress(uint8_t key_id);

/**
 * @brief  获取按键事件总计数
 * @return 累计按键短按次数（所有按键合计）
 */
uint32_t Key_GetEventCount(void);

/**
 * @brief  设置按键消抖时间
 * @param  debounce_ms  消抖时间（毫秒，需与扫描周期对应）
 * @note   默认消抖 20ms，扫描周期 10ms 则需要 2 次确认
 */
void Key_SetDebounceTime(uint8_t debounce_ms);

#ifdef __cplusplus
}
#endif

#endif /* __COMPONENT_KEY_H */
