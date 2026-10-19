/**
 *********************************************************************
 * @file    component_key.c
 * @brief   按键消抖组件实现
 * @note    状态机式消抖：IDLE -> DEBOUNCE -> PRESSED -> RELEASE -> IDLE
 *          扫描周期建议 10ms，消抖确认次数 = 消抖时间 / 扫描周期
 *          一次按下只产生一次短按事件（在 PRESSED 进入瞬间触发）
 *********************************************************************
 */
#include "component_key.h"
#include "bsp_key.h"

/* ---- 按键消抖状态 ---- */
typedef enum {
    KEY_STAGE_IDLE = 0,       //空闲，等待按下
    KEY_STAGE_DEBOUNCE = 1,   //消抖确认中
    KEY_STAGE_PRESSED = 2,    //已按下，等待松开
    KEY_STAGE_RELEASE = 3     //松开确认中
} KeyStage;

/* ---- 单个按键的状态 ---- */
typedef struct {
    KeyStage stage;       //当前状态
    uint8_t  debounce_cnt; //消抖计数器
    uint8_t  short_press;  //短按事件标志（轮询用）
} KeyState;

/* ---- 全局变量 ---- */
static KeyState s_keys[KEY_NUM];            //各按键状态
static KeyEventCallback s_event_cb = 0;      //事件回调
static uint8_t  s_debounce_times = 2;        //消抖确认次数（默认 2 次 = 20ms @10ms 扫描）
static uint32_t s_event_count = 0;           //累计事件数

/**
 * @brief  初始化按键组件
 * @param  cb  事件回调函数
 */
void Key_Init(KeyEventCallback cb)
{
    uint8_t i;
    for (i = 0; i < KEY_NUM; i++) {
        s_keys[i].stage = KEY_STAGE_IDLE;
        s_keys[i].debounce_cnt = 0;
        s_keys[i].short_press = 0;
    }
    s_event_cb = cb;
    s_event_count = 0;

    BSP_Key_Init();
}

/**
 * @brief  设置消抖时间
 * @param  debounce_ms  消抖时间（毫秒）
 * @note   假设扫描周期为 10ms，则次数 = debounce_ms / 10
 */
void Key_SetDebounceTime(uint8_t debounce_ms)
{
    if (debounce_ms < 10) debounce_ms = 10;
    s_debounce_times = debounce_ms / 10;
    if (s_debounce_times < 1) s_debounce_times = 1;
}

/**
 * @brief  获取按键事件总计数
 */
uint32_t Key_GetEventCount(void)
{
    return s_event_count;
}

/**
 * @brief  读取并清除某按键的短按事件（轮询方式）
 * @param  key_id  按键编号
 * @return 1 = 有事件，0 = 无
 */
uint8_t Key_GetShortPress(uint8_t key_id)
{
    uint8_t ret = 0;
    if (key_id < KEY_NUM) {
        ret = s_keys[key_id].short_press;
        s_keys[key_id].short_press = 0;
    }
    return ret;
}

/**
 * @brief  触发一次按键事件（内部调用）
 */
static void Key_TriggerEvent(uint8_t key_id, KeyEventType event)
{
    s_event_count++;
    s_keys[key_id].short_press = 1;

    if (s_event_cb != 0) {
		s_event_cb((KeyID)key_id, (KeyLevel)event);
    }
}

/**
 * @brief  单个按键的状态机处理
 * @param  key_id    按键编号
 * @param  pressed   当前是否按下（1=按下, 0=松开）
 */
static void Key_ProcessOne(uint8_t key_id, uint8_t pressed)
{
    KeyState *k = &s_keys[key_id];

    switch (k->stage) {
    case KEY_STAGE_IDLE:
        if (pressed) {
            k->debounce_cnt = 0;
            k->stage = KEY_STAGE_DEBOUNCE;
        }
        break;

    case KEY_STAGE_DEBOUNCE:
        if (pressed) {
            k->debounce_cnt++;
            if (k->debounce_cnt >= s_debounce_times) {
                /* 确认按下，触发短按事件 */
                k->stage = KEY_STAGE_PRESSED;
                Key_TriggerEvent(key_id, KEY_EVT_SHORT);
            }
        } else {
            /* 抖动，回到空闲 */
            k->stage = KEY_STAGE_IDLE;
        }
        break;

    case KEY_STAGE_PRESSED:
        if (!pressed) {
            /* 开始松开确认 */
            k->debounce_cnt = 0;
            k->stage = KEY_STAGE_RELEASE;
        }
        break;

    case KEY_STAGE_RELEASE:
        if (!pressed) {
            k->debounce_cnt++;
            if (k->debounce_cnt >= s_debounce_times) {
                /* 确认松开，回到空闲 */
                k->stage = KEY_STAGE_IDLE;
            }
        } else {
            /* 又按下了，回到按下状态 */
            k->stage = KEY_STAGE_PRESSED;
        }
        break;

    default:
        k->stage = KEY_STAGE_IDLE;
        break;
    }
}

/**
 * @brief  按键扫描（周期调用，建议 10ms 一次）
 */
void Key_Scan(void)
{
    uint8_t i;
    for (i = 0; i < KEY_NUM; i++) {
        uint8_t pressed = (BSP_Key_ReadLevel((KeyID)i) == KEY_PRESSED) ? 1 : 0;
        Key_ProcessOne(i, pressed);
    }
}
