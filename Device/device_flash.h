/**
 *********************************************************************
 * @file    device_flash.h
 * @brief   内部 Flash 读写驱动头文件
 * @note    Device 层，封装 STM32F1 内部 Flash 操作
 *          用于保存参数到 Flash，掉电不丢失
 *          使用最后 1 页（Page 31，地址 0x0800FC00，1KB）
 *********************************************************************
 */
#ifndef __DEVICE_FLASH_H
#define __DEVICE_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- Flash 参数页配置 ---- */
/* STM32F103C8T6: 64KB Flash, 每页 1KB, 共 64 页 */
/* 使用最后 1 页存参数: Page 63 = 0x0800FC00 */
#define FLASH_PARAM_PAGE_ADDR   0x0800FC00    /* 参数页起始地址 */
#define FLASH_PARAM_PAGE_SIZE   0x0400        /* 每页 1024 字节 */
#define FLASH_MAGIC             0xA5A55A5A    /* 有效标志 */

/* ---- 驱动接口 ---- */

/**
 * @brief  读取 Flash 指定地址的字（32位）
 * @param  addr  Flash 地址（必须字对齐）
 * @return 读取到的 32 位数据
 */
uint32_t Device_Flash_ReadWord(uint32_t addr);

/**
 * @brief  读取一段数据
 * @param  addr   Flash 起始地址
 * @param  buf    数据缓冲区
 * @param  len    数据长度（字节）
 */
void Device_Flash_Read(uint32_t addr, uint8_t *buf, uint16_t len);

/**
 * @brief  擦除指定页
 * @param  page_addr  页起始地址
 * @return HAL_OK / HAL_ERROR
 */
HAL_StatusTypeDef Device_Flash_ErasePage(uint32_t page_addr);

/**
 * @brief  写入一个字（32位）
 * @param  addr   Flash 地址（必须字对齐）
 * @param  data   32 位数据
 * @return HAL_OK / HAL_ERROR
 * @note   写入前必须确保该位置已擦除（全 0xFF）
 */
HAL_StatusTypeDef Device_Flash_WriteWord(uint32_t addr, uint32_t data);

/**
 * @brief  写入一段数据（自动擦除整页）
 * @param  page_addr  页起始地址
 * @param  buf        数据缓冲区
 * @param  len        数据长度（字节，必须 <= 页大小）
 * @return HAL_OK / HAL_ERROR
 * @note   会先擦除整页再写入，调用前请确认数据完整
 */
HAL_StatusTypeDef Device_Flash_WritePage(uint32_t page_addr, const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_FLASH_H */
