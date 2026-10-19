/**
 *********************************************************************
 * @file    task_scheduler.c
 * @brief   任务调度器实现
 * @note    协作式调度：每个任务记录上次执行时间，到点就执行
 *          任务周期配置：
 *            TASK_KEY_SCAN    —— 10ms  （按键扫描 + 消抖）
 *            TASK_LED_PROCESS —— 10ms  （LED 闪烁处理）
 *            TASK_CMD_PROCESS —— 0ms   （每次循环，串口命令处理）
 *            TASK_DISPLAY     —— 100ms （OLED 显示刷新）
 *            TASK_1HZ         —— 1000ms（秒级计时，系统信息）
 *********************************************************************
 */
#include "task_scheduler.h"
#include "component_key.h"
#include "module_led.h"
#include "module_cmd.h"
#include "module_display.h"
#include "module_sysinfo.h"

/* ---- 任务控制块 ---- */
typedef struct {
    uint32_t period_ms;    /* 周期（毫秒），0 表示每次都执行 */
    uint32_t last_tick;    /* 上次执行时间 */
    void (*handler)(void); /* 任务处理函数 */
} TaskCtrl;

/* ---- 任务处理函数声明 ---- */
static void Task_KeyScanHandler(void);
static void Task_LEDProcessHandler(void);
static void Task_CmdProcessHandler(void);
static void Task_DisplayHandler(void);
static void Task_1HzHandler(void);

/* ---- 任务表 ---- */
static TaskCtrl s_tasks[TASK_NUM] = {
    [TASK_KEY_SCAN]    = { 10,   0, Task_KeyScanHandler },
    [TASK_LED_PROCESS] = { 10,   0, Task_LEDProcessHandler },
    [TASK_CMD_PROCESS] = { 0,    0, Task_CmdProcessHandler },
    [TASK_DISPLAY]     = { 100,  0, Task_DisplayHandler },
    [TASK_1HZ]         = { 1000, 0, Task_1HzHandler },
};

/* ============================================================
 * 各任务处理函数
 * ============================================================ */

/**
 * @brief  按键扫描任务（10ms）
 */
static void Task_KeyScanHandler(void)
{
    Key_Scan();
}

/**
 * @brief  LED 处理任务（10ms）
 */
static void Task_LEDProcessHandler(void)
{
    Module_LED_Process();
}

/**
 * @brief  命令处理任务（每次循环）
 */
static void Task_CmdProcessHandler(void)
{
    Module_Cmd_Process();
}

/**
 * @brief  显示刷新任务（100ms）
 */
static void Task_DisplayHandler(void)
{
    Module_Display_Process();
}

/**
 * @brief  1Hz 任务（每秒执行一次）
 */
static void Task_1HzHandler(void)
{
    Module_SysInfo_TickSecond();
}

/* ============================================================
 * 调度器公共接口
 * ============================================================ */

/**
 * @brief  调度器初始化
 */
void Task_SchedulerInit(void)
{
    uint8_t i;
    uint32_t now = HAL_GetTick();
    for (i = 0; i < TASK_NUM; i++) {
        s_tasks[i].last_tick = now;
    }
}

/**
 * @brief  调度器主循环
 */
void Task_SchedulerRun(void)
{
    uint8_t i;
    uint32_t now = HAL_GetTick();

    for (i = 0; i < TASK_NUM; i++) {
        TaskCtrl *t = &s_tasks[i];

        if (t->period_ms == 0) {
            /* 周期为 0 表示每次都执行 */
            if (t->handler != 0) {
                t->handler();
            }
        } else {
            /* 周期任务：检查是否到期 */
            if ((now - t->last_tick) >= t->period_ms) {
                t->last_tick = now;
                if (t->handler != 0) {
                    t->handler();
                }
            }
        }
    }
}
