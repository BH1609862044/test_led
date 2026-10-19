/**
 *********************************************************************
 * @file    device_oled.h
 * @brief   0.96 寸 OLED (SSD1306) 设备驱动头文件
 * @note    I2C 接口，分辨率 128x64
 *          设备层只负责显示驱动，不包含业务逻辑
 *********************************************************************
 */
#ifndef __DEVICE_OLED_H
#define __DEVICE_OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- OLED 参数 ---- */
#define OLED_I2C_ADDR    0x78   /* OLED 设备地址（8 位写地址，常见 0x3C << 1 = 0x78） */
#define OLED_WIDTH       128    /* 宽度 128 像素 */
#define OLED_HEIGHT      64     /* 高度 64 像素 */
#define OLED_PAGES       8      /* 共 8 页（每页 8 行） */

/* ---- OLED 操作接口 ---- */
void OLED_Init(void);                          /* OLED 初始化 */
void OLED_Clear(void);                         /* 清屏 */
void OLED_SetPos(uint8_t page, uint8_t col);   /* 设置光标位置（页地址, 列地址） */

/* ---- 字符/字符串显示 ---- */
void OLED_ShowChar(uint8_t page, uint8_t col, char ch);       /* 在指定位置显示一个字符（6x8） */
void OLED_ShowString(uint8_t page, uint8_t col, const char *str);  /* 在指定位置显示字符串 */
void OLED_ShowNum(uint8_t page, uint8_t col, uint32_t num, uint8_t len); /* 显示数字（指定长度） */

/* ---- 全屏刷新（使用显存模式时调用）---- */
void OLED_UpdateScreen(void);                  /* 将显存内容刷新到 OLED（可选接口） */

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_OLED_H */
