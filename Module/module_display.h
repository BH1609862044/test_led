/**
 *********************************************************************
 * @file    module_display.h
 * @brief   OLED 显示模块头文件（多级菜单）
 * @note    Module 层，负责 OLED 界面的组织和刷新
 *          5 个页面：首页 / LED控制 / 闪烁参数 / 系统信息 / 命令说明
 *          显示数据来自 Module 层，不直接操作硬件
 *********************************************************************
 */
#ifndef __MODULE_DISPLAY_H
#define __MODULE_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 页面类型 ---- */
typedef enum {
    PAGE_BOOT = 0,        /* 启动页 */
    PAGE_HOME = 1,        /* 第1页：首页 */
    PAGE_LED_CTRL = 2,    /* 第2页：LED 控制 */
    PAGE_BLINK_PARAM = 3, /* 第3页：闪烁参数 */
    PAGE_SYSINFO = 4,     /* 第4页：系统信息 */
    PAGE_CMD_HELP = 5,    /* 第5页：命令说明 */
    PAGE_MAX
} PageType;

/* ---- 菜单操作 ---- */
typedef enum {
    MENU_KEY_NEXT = 0,    /* KEY1 短按：下一页 / 向下选择 */
    MENU_KEY_PREV = 1,    /* KEY1 长按：回到首页 */
    MENU_KEY_INC  = 2,    /* KEY2 短按：加 / 确认 */
    MENU_KEY_DEC  = 3,    /* KEY3 短按：减 / 返回 */
    MENU_KEY_INC_HOLD = 4,/* KEY2 连续触发：快速加 */
    MENU_KEY_DEC_HOLD = 5,/* KEY3 连续触发：快速减 */
    MENU_KEY_HOME = 6     /* KEY1 长按：回到首页 */
} MenuKeyOp;

/* ---- 模块接口 ---- */

/**
 * @brief  显示模块初始化
 */
void Module_Display_Init(void);

/**
 * @brief  显示启动页（上电自检）
 */
void Module_Display_ShowBootPage(void);

/**
 * @brief  切换到首页
 */
void Module_Display_ShowHomePage(void);

/**
 * @brief  菜单按键输入（App 层按键回调中调用）
 * @param  op  按键操作
 * @note   各页面根据按键操作做不同响应
 */
void Module_Display_MenuKey(MenuKeyOp op);

/**
 * @brief  获取当前页面
 * @return 当前页面编号
 */
PageType Module_Display_GetCurrentPage(void);

/**
 * @brief  显示模块周期刷新（建议 100ms 调用一次）
 * @note   内部刷新运行时间等动态信息
 */
void Module_Display_Process(void);

/**
 * @brief  显示错误提示（临时在屏幕底部显示）
 * @param  err_str  错误字符串
 */
void Module_Display_ShowError(const char *err_str);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_DISPLAY_H */
