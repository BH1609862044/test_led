/**
 *********************************************************************
 * @file    device_flash.c
 * @brief   内部 Flash 读写驱动实现
 *********************************************************************
 */
#include "device_flash.h"

/**
 * @brief  读取 Flash 指定地址的字
 */
uint32_t Device_Flash_ReadWord(uint32_t addr)
{
    return (*(__IO uint32_t *)addr);
}

/**
 * @brief  读取一段数据
 */
void Device_Flash_Read(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++) {
        buf[i] = *(__IO uint8_t *)(addr + i);
    }
}

/**
 * @brief  擦除指定页
 */
HAL_StatusTypeDef Device_Flash_ErasePage(uint32_t page_addr)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_init.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = page_addr;
    erase_init.NbPages     = 1;

    status = HAL_FLASHEx_Erase(&erase_init, &page_error);

    HAL_FLASH_Lock();
    return status;
}

/**
 * @brief  写入一个字（32位）
 */
HAL_StatusTypeDef Device_Flash_WriteWord(uint32_t addr, uint32_t data)
{
    HAL_StatusTypeDef status;

    HAL_FLASH_Unlock();
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);
    HAL_FLASH_Lock();

    return status;
}

/**
 * @brief  写入一段数据（先擦除整页，再逐字写入）
 */
HAL_StatusTypeDef Device_Flash_WritePage(uint32_t page_addr, const uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint16_t i;
    uint32_t word_data;

    if (buf == 0 || len == 0 || len > FLASH_PARAM_PAGE_SIZE) {
        return HAL_ERROR;
    }

    /* 1. 先擦除整页 */
    status = Device_Flash_ErasePage(page_addr);
    if (status != HAL_OK) {
        return status;
    }

    /* 2. 解锁 Flash */
    HAL_FLASH_Unlock();

    /* 3. 按字写入（32位对齐） */
    for (i = 0; i < len; i += 4) {
        word_data = 0xFFFFFFFF;
        /* 小端模式：低字节在低地址 */
        if (i < len) {
            word_data = (word_data & 0xFFFFFF00UL) | buf[i];
        }
        if (i + 1 < len) {
            word_data = (word_data & 0xFFFF00FFUL) | ((uint32_t)buf[i + 1] << 8);
        }
        if (i + 2 < len) {
            word_data = (word_data & 0xFF00FFFFUL) | ((uint32_t)buf[i + 2] << 16);
        }
        if (i + 3 < len) {
            word_data = (word_data & 0x00FFFFFFUL) | ((uint32_t)buf[i + 3] << 24);
        }

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                   page_addr + i, word_data);
        if (status != HAL_OK) {
            break;
        }
    }

    /* 4. 上锁 */
    HAL_FLASH_Lock();
    return status;
}
