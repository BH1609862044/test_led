/**
 *********************************************************************
 * @file    module_cmd.c
 * @brief   串口命令解析模块实现
 * @note    支持命令：
 *          基础题：HELP / LED ON/OFF/TOGGLE / STATUS
 *          提高题：GET STATUS / GET PARAM / SET MODE / SET PERIOD
 *                  SET LED / SAVE / RESET
 *          错误处理：未知命令 / 参数越界 / 空行 / 超长
 *********************************************************************
 */
#include "module_cmd.h"
#include "device_uart.h"
#include "module_led.h"
#include "module_params.h"
#include "module_sysinfo.h"
#include "component_key.h"
#include "bsp_uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ---- 命令行缓冲区 ---- */
static char s_line_buf[128];

/* ---- 私有函数声明 ---- */
static void trim(char *s);
static void strupr(char *s);
static int  strstart(const char *s, const char *prefix);
static void send_ok(const char *msg);
static void send_error(const char *msg);
static void cmd_help(void);
static void cmd_led(const char *param);
static void cmd_status(void);
static void cmd_get_status(void);
static void cmd_get_param(void);
static void cmd_set_mode(const char *param);
static void cmd_set_period(const char *param);
static void cmd_set_led(const char *param);
static void cmd_save(void);
static void cmd_reset(void);

/**
 * @brief  初始化
 */
void Module_Cmd_Init(void)
{
    /* 什么都不用做，设备层已经初始化好了 */
}

/**
 * @brief  命令处理（轮询）
 */
void Module_Cmd_Process(void)
{
    uint16_t len;

    if (!Device_UART_IsLineReady()) {
        return;
    }

    len = Device_UART_ReadLine(s_line_buf, sizeof(s_line_buf));

    if (len == UART_RX_LINE_TOO_LONG) {
        send_error("COMMAND TOO LONG");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("TOO LONG");
        return;
    }

    if (len == UART_RX_QUEUE_FULL) {
        send_error("COMMAND QUEUE FULL");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("QUEUE FULL");
        return;
    }

    if (len == 0) {
        /* 空行 */
        send_error("EMPTY COMMAND");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("EMPTY CMD");
        return;
    }

    if (len >= UART_RX_BUF_SIZE) {
        send_error("COMMAND TOO LONG");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("TOO LONG");
        return;
    }

    /* 去首尾空格 */
    trim(s_line_buf);

    if (s_line_buf[0] == '\0') {
        send_error("EMPTY COMMAND");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("EMPTY CMD");
        return;
    }

    /* 转大写（只转命令部分，参数保持原样但这里都转也没关系） */
    strupr(s_line_buf);

    /* ---- 解析命令 ---- */

    if (strcmp(s_line_buf, "HELP") == 0) {
        cmd_help();
    }
    else if (strstart(s_line_buf, "LED ")) {
        cmd_led(s_line_buf + 4);
    }
    else if (strcmp(s_line_buf, "STATUS") == 0) {
        cmd_status();
    }
    else if (strcmp(s_line_buf, "GET STATUS") == 0) {
        cmd_get_status();
    }
    else if (strcmp(s_line_buf, "GET PARAM") == 0) {
        cmd_get_param();
    }
    else if (strstart(s_line_buf, "SET MODE ")) {
        cmd_set_mode(s_line_buf + 9);
    }
    else if (strstart(s_line_buf, "SET PERIOD ")) {
        cmd_set_period(s_line_buf + 11);
    }
    else if (strstart(s_line_buf, "SET LED ")) {
        cmd_set_led(s_line_buf + 8);
    }
    else if (strcmp(s_line_buf, "SAVE") == 0) {
        cmd_save();
    }
    else if (strcmp(s_line_buf, "RESET") == 0) {
        cmd_reset();
    }
    else {
        send_error("UNKNOWN COMMAND");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("UNKNOWN CMD");
    }
}

/* ========== 命令处理函数 ========== */

/**
 * @brief  HELP 命令
 */
static void cmd_help(void)
{
    BSP_UART_SendString("=== Command List ===\r\n");
    BSP_UART_SendString(" HELP               Show this help\r\n");
    BSP_UART_SendString(" LED ON/OFF/TOGGLE  Control LED (basic)\r\n");
    BSP_UART_SendString(" STATUS             Show basic status\r\n");
    BSP_UART_SendString(" GET STATUS         Show full status\r\n");
    BSP_UART_SendString(" GET PARAM          Show saved parameters\r\n");
    BSP_UART_SendString(" SET MODE 0/1/2     0=MANUAL 1=BLINK 2=FAST\r\n");
    BSP_UART_SendString(" SET PERIOD N       Set blink period(100-5000ms)\r\n");
    BSP_UART_SendString(" SET LED ON/OFF     Set LED state\r\n");
    BSP_UART_SendString(" SAVE               Save params to Flash\r\n");
    BSP_UART_SendString(" RESET              Reset params to default\r\n");
    BSP_UART_SendString("====================\r\n");
    Module_SysInfo_IncCmdOk();
}

/**
 * @brief  LED 命令（基础题兼容）
 */
static void cmd_led(const char *param)
{
    if (strcmp(param, "ON") == 0) {
        Module_LED_SetState(LED_ON);
        Module_Params_SetLedState(1);
        send_ok("LED=ON");
        Module_SysInfo_IncCmdOk();
    }
    else if (strcmp(param, "OFF") == 0) {
        Module_LED_SetState(LED_OFF);
        Module_Params_SetLedState(0);
        send_ok("LED=OFF");
        Module_SysInfo_IncCmdOk();
    }
    else if (strcmp(param, "TOGGLE") == 0) {
        Module_LED_Toggle();
        if (Module_LED_GetState() == LED_ON) {
            send_ok("LED=ON");
        } else {
            send_ok("LED=OFF");
        }
        Module_SysInfo_IncCmdOk();
    }
    else {
        send_error("BAD LED STATE");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("BAD LED");
    }
}

/**
 * @brief  STATUS 命令（基础题版本）
 */
static void cmd_status(void)
{
    char buf[64];
    LedMode mode = Module_LED_GetMode();
    const char *mode_str = Module_LED_GetModeName(mode);
    const char *led_str = (Module_LED_GetState() == LED_ON) ? "ON" : "OFF";
    uint32_t count = Key_GetEventCount();

    snprintf(buf, sizeof(buf),
             "MODE=%s LED=%s COUNT=%04lu\r\n",
             mode_str, led_str, (unsigned long)count);
    BSP_UART_SendString(buf);
    Module_SysInfo_IncCmdOk();
}

/**
 * @brief  GET STATUS 命令（完整状态）
 */
static void cmd_get_status(void)
{
    char buf[128];
    LedMode mode = Module_LED_GetMode();
    const char *mode_str = Module_LED_GetModeName(mode);
    const char *led_str = (Module_LED_GetState() == LED_ON) ? "ON" : "OFF";
    uint16_t period = (mode == LED_MODE_FAST_BLINK) ?
                       Module_LED_GetFastPeriod() : Module_LED_GetBlinkPeriod();
    uint32_t uptime = Module_SysInfo_GetUptime();
    uint32_t key_cnt = Key_GetEventCount();
    uint32_t cmd_ok = Module_SysInfo_GetCmdOkCount();
    uint32_t cmd_err = Module_SysInfo_GetCmdErrCount();
    const char *last_err = Module_SysInfo_GetLastError();

    snprintf(buf, sizeof(buf),
             "=== System Status ===\r\n"
             " SW Version: %s\r\n"
             " Uptime: %lus\r\n"
             " LED Mode: %s\r\n"
             " LED State: %s\r\n"
             " Period: %ums\r\n"
             " Key Count: %lu\r\n"
             " Cmd OK: %lu\r\n"
             " Cmd Err: %lu\r\n"
             " Last Error: %s\r\n"
             "=====================\r\n",
             Module_SysInfo_GetSwVersion(),
             (unsigned long)uptime,
             mode_str, led_str,
             (unsigned)period,
             (unsigned long)key_cnt,
             (unsigned long)cmd_ok,
             (unsigned long)cmd_err,
             last_err);
    BSP_UART_SendString(buf);
    Module_SysInfo_IncCmdOk();
}

/**
 * @brief  GET PARAM 命令
 */
static void cmd_get_param(void)
{
    char buf[96];
    const ParamsStruct *p = Module_Params_Get();

    snprintf(buf, sizeof(buf),
             "=== Parameters ===\r\n"
             " MODE: %u (%s)\r\n"
             " BLINK PERIOD: %ums\r\n"
             " FAST PERIOD: %ums\r\n"
             " DEFAULT LED: %s\r\n"
             " PARAM VERSION: 0x%04X\r\n"
             "==================\r\n",
             (unsigned)p->led_mode,
             Module_LED_GetModeName((LedMode)p->led_mode),
             (unsigned)p->blink_period_ms,
             (unsigned)p->fast_period_ms,
             p->led_state ? "ON" : "OFF",
             (unsigned)p->version);
    BSP_UART_SendString(buf);
    Module_SysInfo_IncCmdOk();
}

/**
 * @brief  SET MODE 命令
 */
static void cmd_set_mode(const char *param)
{
    char *end;
    long mode_num = strtol(param, &end, 10);

    if (end == param || *end != '\0' || mode_num < 0 || mode_num > 2) {
        send_error("MODE RANGE");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("MODE RANGE");
        return;
    }

    Module_LED_SetMode((LedMode)mode_num);
    Module_Params_SetLedMode((uint8_t)mode_num);

    {
        char buf[32];
        snprintf(buf, sizeof(buf), "MODE=%s",
                 Module_LED_GetModeName((LedMode)mode_num));
        send_ok(buf);
    }
    Module_SysInfo_IncCmdOk();
}

/**
 * @brief  SET PERIOD 命令
 */
static void cmd_set_period(const char *param)
{
    char *end;
    long parsed_period = strtol(param, &end, 10);
    uint32_t period;
    char buf[32];

    if (end == param || *end != '\0' ||
        parsed_period < PARAM_BLINK_PERIOD_MIN ||
        parsed_period > PARAM_BLINK_PERIOD_MAX) {
        send_error("PERIOD RANGE");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("PERIOD RANGE");
        return;
    }
    period = (uint32_t)parsed_period;

    /* 设置当前模式对应的周期 */
    if (Module_LED_GetMode() == LED_MODE_FAST_BLINK) {
        Module_LED_SetFastPeriod((uint16_t)period);
        Module_Params_SetFastPeriod((uint16_t)period);
    } else {
        Module_LED_SetBlinkPeriod((uint16_t)period);
        Module_Params_SetBlinkPeriod((uint16_t)period);
    }

    snprintf(buf, sizeof(buf), "PERIOD=%ums", (unsigned)period);
    send_ok(buf);
    Module_SysInfo_IncCmdOk();
}

/**
 * @brief  SET LED 命令
 */
static void cmd_set_led(const char *param)
{
    if (strcmp(param, "ON") == 0) {
        Module_LED_SetState(LED_ON);
        Module_Params_SetLedState(1);
        send_ok("LED=ON");
        Module_SysInfo_IncCmdOk();
    }
    else if (strcmp(param, "OFF") == 0) {
        Module_LED_SetState(LED_OFF);
        Module_Params_SetLedState(0);
        send_ok("LED=OFF");
        Module_SysInfo_IncCmdOk();
    }
    else {
        send_error("BAD LED STATE");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("BAD LED");
    }
}

/**
 * @brief  SAVE 命令
 */
static void cmd_save(void)
{
    HAL_StatusTypeDef status;

    /* 把当前 LED 状态也同步一下 */
    Module_Params_SetLedMode((uint8_t)Module_LED_GetMode());
    Module_Params_SetLedState((uint8_t)Module_LED_GetState());

    status = Module_Params_Save();
    if (status == HAL_OK) {
        send_ok("SAVE");
        Module_SysInfo_IncCmdOk();
    } else {
        send_error("SAVE FAIL");
        Module_SysInfo_IncCmdErr();
        Module_SysInfo_SetLastError("SAVE FAIL");
    }
}

/**
 * @brief  RESET 命令（恢复默认参数）
 */
static void cmd_reset(void)
{
    Module_Params_LoadDefault();

    /* 应用默认参数 */
    Module_LED_SetMode((LedMode)Module_Params_Get()->led_mode);
    Module_LED_SetBlinkPeriod(Module_Params_Get()->blink_period_ms);
    Module_LED_SetFastPeriod(Module_Params_Get()->fast_period_ms);
    if (Module_Params_Get()->led_state) {
        Module_LED_SetState(LED_ON);
    } else {
        Module_LED_SetState(LED_OFF);
    }

    Module_SysInfo_ResetStats();

    send_ok("RESET");
    Module_SysInfo_IncCmdOk();
}

/* ========== 工具函数 ========== */

/**
 * @brief  去除字符串首尾空格
 */
static void trim(char *s)
{
    char *start = s;
    char *end;

    /* 跳过头空格 */
    while (*start && isspace((unsigned char)*start)) start++;

    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    if (*s == '\0') {
        return;
    }

    /* 去掉尾空格 */
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
}

/**
 * @brief  字符串转大写
 */
static void strupr(char *s)
{
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

/**
 * @brief  检查字符串是否以指定前缀开头
 * @return 1 = 是，0 = 否
 */
static int strstart(const char *s, const char *prefix)
{
    while (*prefix) {
        if (*s == '\0' || *s != *prefix) return 0;
        s++;
        prefix++;
    }
    return 1;
}

/**
 * @brief  发送 OK 响应
 */
static void send_ok(const char *msg)
{
    BSP_UART_SendString("OK ");
    BSP_UART_SendString(msg);
    BSP_UART_SendString("\r\n");
}

/**
 * @brief  发送 ERROR 响应
 */
static void send_error(const char *msg)
{
    BSP_UART_SendString("ERROR: ");
    BSP_UART_SendString(msg);
    BSP_UART_SendString("\r\n");
}
