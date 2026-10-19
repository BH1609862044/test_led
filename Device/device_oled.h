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
#define OLED_I2C_ADDR    0x78   /* OLED 设备地址（8 位写地址，0x3C << 1 = 0x78） */
#define OLED_WIDTH       128    /* 宽度 128 像素 */
#define OLED_HEIGHT      64     /* 高度 64 像素 */
#define OLED_PAGES       8      /* 共 8 页（每页 8 行） */

/* ---- OLED 操作接口 ---- */

/**
 * @brief  OLED 初始化
 */
void Device_OLED_Init(void);

/**
 * @brief  测试 OLED 是否连接正常
 * @return 1 = 正常，0 = 无响应
 */
uint8_t Device_OLED_TestConnection(void);

/**
 * @brief  清屏
 */
void Device_OLED_Clear(void);

/**
 * @brief  设置光标位置
 * @param  page  页地址（0~7）
 * @param  col   列地址（0~127）
 */
void Device_OLED_SetPos(uint8_t page, uint8_t col);

/**
 * @brief  刷新 OLED 显示（有显存模式下使用）
 * @note   当前驱动为直接写屏模式，此函数留作兼容
 */
void Device_OLED_Refresh(void);

/* ---- 字符/字符串显示 ---- */

/**
 * @brief  在指定位置显示一个字符（6x8 字模）
 * @param  page  页（行，0~7）
 * @param  col   列（0~127）
 * @param  ch    字符
 */
void Device_OLED_ShowChar(uint8_t page, uint8_t col, char ch);

/**
 * @brief  在指定位置显示字符串
 * @param  page  页（行，0~7）
 * @param  col   起始列
 * @param  str   字符串
 */
void Device_OLED_ShowString(uint8_t page, uint8_t col, const char *str);

/**
 * @brief  显示数字（指定长度）
 * @param  page  页
 * @param  col   起始列
 * @param  num   数字
 * @param  len   显示长度（位数，不足前面补 0）
 */
void Device_OLED_ShowNum(uint8_t page, uint8_t col, uint32_t num, uint8_t len);

/* ---- 兼容旧命名（保留 OLED_ 前缀） ---- */
#define OLED_Init()            Device_OLED_Init()
#define OLED_Clear()           Device_OLED_Clear()
#define OLED_SetPos(p,c)       Device_OLED_SetPos(p,c)
#define OLED_ShowChar(p,c,ch)  Device_OLED_ShowChar(p,c,ch)
#define OLED_ShowString(p,c,s) Device_OLED_ShowString(p,c,s)
#define OLED_ShowNum(p,c,n,l)  Device_OLED_ShowNum(p,c,n,l)
#define OLED_Refresh()         Device_OLED_Refresh()

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_OLED_H */
