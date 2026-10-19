/**
 *********************************************************************
 * @file    bsp_key.h
 * @brief   板载按键板级封装头文件
 * @note    只负责读取 GPIO 电平，不做消抖和事件判断
 *********************************************************************
 */
#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* ---- 按键编号 ---- */
typedef enum {
    KEY_1 = 0,   // KEY1
    KEY_2 = 1,   // KEY2 按键
    KEY_3 = 2,   // KEY3 按键
    KEY_ID_COUNT // 按键总数
} KeyID;

/* ---- 按键电平状态 ---- */
typedef enum {
    KEY_RELEASED = 0,  // 按键松开
    KEY_PRESSED  = 1   // 按键按下
} KeyLevel;

/* ---- 按键读取接口 ---- */
void    BSP_Key_Init(void);      // 初始化按键 GPIO（CubeMX 已配置，此处可选）
KeyLevel BSP_Key_ReadLevel(KeyID id);  // 读取按键原始电平（上拉输入时，按下为低电平）

#ifdef __cplusplus
}
#endif

#endif /* __BSP_KEY_H */
