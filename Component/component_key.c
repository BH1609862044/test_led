/**
 *********************************************************************
 * @file    component_key.c
 * @brief   按键消抖组件实现
 * @note    状态机式消抖 + 长按检测 + 连续触发
 *          扫描周期建议 10ms
 *********************************************************************
 */
#include "component_key.h"
#include "bsp_key.h"
#include <string.h>

/* ---- 按键状态 ---- */
typedef enum {
    KEY_STATE_IDLE = 0,      /* 空闲：松开 */
    KEY_STATE_DEBOUNCE,      /* 消抖确认中：刚检测到按下 */
    KEY_STATE_PRESSED,       /* 已按下（稳定） */
    KEY_STATE_LONG_TRIGGERED,/* 已触发长按事件 */
    KEY_STATE_RELEASE        /* 释放确认中 */
} KeyState;

/* ---- 单个按键数据 ---- */
typedef struct {
    KeyState   state;        /* 当前状态 */
    uint8_t    debounce_cnt; /* 消抖计数器（单位：扫描次数） */
    uint16_t   hold_cnt;     /* 按住时间计数器（单位：扫描次数） */
    uint16_t   repeat_cnt;   /* 连续触发计数器 */
    uint8_t    short_flag;   /* 短按事件标志（轮询用） */
    uint8_t    long_flag;    /* 长按事件标志（轮询用） */
    uint8_t    long_fired;   /* 本次按下已触发长按 */
} KeyData;

/* ---- 私有变量 ---- */
static KeyData s_keys[KEY_NUM];
static KeyEventCallback s_callback = NULL;
static uint32_t s_event_count = 0;

/* 配置参数（单位：扫描次数，默认扫描周期 10ms） */
static uint8_t  s_debounce_cnt = 2;   /* 消抖 2 次 = 20ms */
static uint16_t s_long_press_cnt = 100; /* 长按 100 次 = 1000ms */
static uint16_t s_repeat_cnt = 20;    /* 连续触发 20 次 = 200ms */

/* ---- 私有函数声明 ---- */
static void process_key(uint8_t key_id, uint8_t is_pressed);

/**
 * @brief  初始化
 */
void Key_Init(KeyEventCallback cb)
{
    memset(s_keys, 0, sizeof(s_keys));
    s_callback = cb;
    s_event_count = 0;
}

/**
 * @brief  扫描所有按键
 */
void Key_Scan(void)
{
    uint8_t i;
    uint8_t level;

    for (i = 0; i < KEY_NUM; i++) {
        level = (BSP_Key_ReadLevel((KeyID)i) == KEY_PRESSED) ? 1 : 0;
        process_key(i, level);
    }
}

/**
 * @brief  单个按键状态机处理
 * @param  key_id       按键编号（0~2）
 * @param  is_pressed   1 = 当前检测到按下，0 = 当前检测到松开
 */
static void process_key(uint8_t key_id, uint8_t is_pressed)
{
    KeyData *k = &s_keys[key_id];

    switch (k->state) {

    /* ---- 空闲状态 ---- */
    case KEY_STATE_IDLE:
        if (is_pressed) {
            k->state = KEY_STATE_DEBOUNCE;
            k->debounce_cnt = 0;
        }
        break;

    /* ---- 按下消抖 ---- */
    case KEY_STATE_DEBOUNCE:
        if (is_pressed) {
            k->debounce_cnt++;
            if (k->debounce_cnt >= s_debounce_cnt) {
                /* 确认按下 */
                k->state = KEY_STATE_PRESSED;
                k->hold_cnt = 0;
                k->repeat_cnt = 0;
                k->long_fired = 0;
            }
        } else {
            /* 抖动，回到空闲 */
            k->state = KEY_STATE_IDLE;
        }
        break;

    /* ---- 已按下（等待长按或释放） ---- */
    case KEY_STATE_PRESSED:
        if (!is_pressed) {
            /* 等待松开消抖后再触发短按 */
            k->state = KEY_STATE_RELEASE;
            k->debounce_cnt = 1;
        } else if (++k->hold_cnt >= s_long_press_cnt) {
            /* 达到长按阈值，触发长按事件 */
            k->long_flag = 1;
            k->long_fired = 1;
            s_event_count++;
            if (s_callback) {
                s_callback(key_id, KEY_EVT_LONG);
            }
            k->state = KEY_STATE_LONG_TRIGGERED;
            k->repeat_cnt = 0;
        }
        break;

    /* ---- 已触发长按（进入连续触发模式） ---- */
    case KEY_STATE_LONG_TRIGGERED:
        if (!is_pressed) {
            /* 等待松开消抖 */
            k->state = KEY_STATE_RELEASE;
            k->debounce_cnt = 1;
        } else if (++k->repeat_cnt >= s_repeat_cnt) {
            /* 连续触发 */
            k->repeat_cnt = 0;
            if (s_callback) {
                s_callback(key_id, KEY_EVT_REPEAT);
            }
            /* 注意：repeat 不计入 event_count，避免计数暴增 */
        }
        break;

    /* ---- 释放消抖 ---- */
    case KEY_STATE_RELEASE:
        if (!is_pressed) {
            k->debounce_cnt++;
            if (k->debounce_cnt >= s_debounce_cnt) {
                if (!k->long_fired) {
                    k->short_flag = 1;
                    s_event_count++;
                    if (s_callback) {
                        s_callback(key_id, KEY_EVT_SHORT);
                    }
                }
                k->state = KEY_STATE_IDLE;
            }
        } else {
            /* 松开抖动，恢复到原来的按下状态 */
            k->state = k->long_fired ? KEY_STATE_LONG_TRIGGERED : KEY_STATE_PRESSED;
        }
        break;

    default:
        k->state = KEY_STATE_IDLE;
        break;
    }
}

/**
 * @brief  读取短按事件（轮询方式）
 */
uint8_t Key_GetShortPress(uint8_t key_id)
{
    if (key_id >= KEY_NUM) return 0;
    if (s_keys[key_id].short_flag) {
        s_keys[key_id].short_flag = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief  读取长按事件（轮询方式）
 */
uint8_t Key_GetLongPress(uint8_t key_id)
{
    if (key_id >= KEY_NUM) return 0;
    if (s_keys[key_id].long_flag) {
        s_keys[key_id].long_flag = 0;
        return 1;
    }
    return 0;
}

/**
 * @brief  获取总事件数
 */
uint32_t Key_GetEventCount(void)
{
    return s_event_count;
}

/**
 * @brief  设置消抖时间（毫秒）
 */
void Key_SetDebounceTime(uint8_t debounce_ms)
{
    /* 扫描周期按 10ms 估算 */
    s_debounce_cnt = (debounce_ms + 9) / 10;
    if (s_debounce_cnt < 1) s_debounce_cnt = 1;
}

/**
 * @brief  设置长按时间（毫秒）
 */
void Key_SetLongPressTime(uint16_t long_press_ms)
{
    s_long_press_cnt = (long_press_ms + 9) / 10;
    if (s_long_press_cnt < 5) s_long_press_cnt = 5;
}

/**
 * @brief  设置连续触发间隔（毫秒）
 */
void Key_SetRepeatInterval(uint16_t repeat_ms)
{
    s_repeat_cnt = (repeat_ms + 9) / 10;
    if (s_repeat_cnt < 2) s_repeat_cnt = 2;
}
