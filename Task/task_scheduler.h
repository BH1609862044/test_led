/**
 *********************************************************************
 * @file    task_scheduler.h
 * @brief   任务调度器头文件
 * @note    简易协作式任务调度器
 *          每个任务有独立的周期，在主循环中依次检查执行
 *          不使用操作系统，纯状态机 + HAL_GetTick() 时基
 *********************************************************************
 */
#ifndef __TASK_SCHEDULER_H
#define __TASK_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ---- 任务 ID 定义 ---- */
typedef enum {
    TASK_KEY_SCAN = 0,       /* 按键扫描任务（10ms） */
    TASK_LED_PROCESS = 1,    /* LED 处理任务（10ms） */
    TASK_CMD_PROCESS = 2,    /* 命令处理任务（主循环轮询） */
    TASK_DISPLAY = 3,        /* 显示刷新任务（100ms） */
    TASK_NUM                 /* 任务总数 */
} TaskID;

/* ---- 调度器接口 ---- */

/**
 * @brief  调度器初始化
 */
void Task_SchedulerInit(void);

/**
 * @brief  调度器主循环（在 main.c 的 while(1) 中调用）
 * @note   内部按各任务周期调度执行
 */
void Task_SchedulerRun(void);

#ifdef __cplusplus
}
#endif

#endif /* __TASK_SCHEDULER_H */
