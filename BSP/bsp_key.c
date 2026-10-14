#include "bsp_key.h"
#include "main.h"
/*
 * 按键：KEY1 -> PA3, KEY2 -> PA4, KEY3 -> PA5
 * CubeMX 里引脚 Label 要对应命名：KEY1、KEY2、KEY3
 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} KeyPinMap;
static const KeyPinMap s_key_map[KEY_NUM] = {
    { KEY1_GPIO_Port, KEY1_Pin },   // KEY1 —— PA3
    { KEY2_GPIO_Port, KEY2_Pin },   // KEY2 —— PA4
    { KEY3_GPIO_Port, KEY3_Pin },   // KEY3 —— PA5
};
/**
 * @brief  按键初始化（CubeMX 已配置 GPIO，此处可选）
 */
void BSP_Key_Init(void)
{
    // CubeMX 已配置 GPIO，此处可选
/**
 * @brief  读取按键原始电平
 * @param  id  按键编号 KEY_1 / KEY_2 / KEY_3
 * @return KEY_PRESSED / KEY_RELEASED
 * @note   上拉输入时，按下为低电平 -> KEY_PRESSED
 */
KeyLevel BSP_Key_ReadLevel(KeyID id)
{
    if (id >= KEY_NUM) {
        return KEY_RELEASED;
    }
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(s_key_map[id].port, s_key_map[id].pin);
    //上拉输入：低电平 = 按下
    if (pin_state == GPIO_PIN_RESET) {
        return KEY_PRESSED;
    } else {
        return KEY_RELEASED;
    }
}
}