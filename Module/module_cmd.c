/**
 *********************************************************************
 * @file    module_cmd.c
 * @brief   串口命令解析模块实现
 * @note    基础题命令列表：
 *          HELP            —— 列出支持的命令
 *          LED ON          —— LED 点亮
 *          LED OFF         —— LED 熄灭
 *          LED TOGGLE      —— LED 翻转
 *          STATUS          —— 返回当前状态
 *
 *          命令大小写不敏感，解析前统一转大写
 *          命令前后空格自动忽略
 *********************************************************************
 */
#include "module_cmd.h"
#include "device_uart.h"
#include "module_led.h"
#include "component_key.h"
#include <string.h>
#include <ctype.h>

/* ---- 统计 ---- */
static uint32_t s_success_count = 0;
static uint32_t s_error_count = 0;
static CmdResult s_last_error = CMD_OK;

/* ---- 内部工具函数 ---- */

/**
 * @brief  去除字符串首尾空格
 * @param  str  字符串（原位修改）
 * @return 处理后的字符串指针
 */
static char *Cmd_Trim(char *str)
{
    char *start = str;
    char *end;

    /* 跳过开头空格 */
    while (*start && (*start == ' ' || *start == '\t')) start++;

    /* 如果全是空格 */
    if (*start == '\0') return start;

    /* 去掉末尾空格 */
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')) end--;
    *(end + 1) = '\0';

    return start;
}

/**
 * @brief  字符串转大写（原位修改）
 */
static void Cmd_ToUpper(char *str)
{
    while (*str) {
        if (*str >= 'a' && *str <= 'z') {
            *str = *str - 'a' + 'A';
        }
        str++;
    }
}

/**
 * @brief  发送成功响应
 */
static void Cmd_ReplyOK(const char *msg)
{
    Device_UART_SendString("OK ");
    Device_UART_SendString(msg);
    Device_UART_SendString("\r\n");
}

/**
 * @brief  发送错误响应
 */
static void Cmd_ReplyError(const char *msg)
{
    Device_UART_SendString("ERROR: ");
    Device_UART_SendString(msg);
    Device_UART_SendString("\r\n");
}

/* ---- 命令处理函数 ---- */

/**
 * @brief  HELP 命令
 */
static void Cmd_HandleHelp(void)
{
    Device_UART_SendString("=== STM32 Control Commands ===\r\n");
    Device_UART_SendString("  HELP            Show this help\r\n");
    Device_UART_SendString("  LED ON          Turn LED on\r\n");
    Device_UART_SendString("  LED OFF         Turn LED off\r\n");
    Device_UART_SendString("  LED TOGGLE      Toggle LED\r\n");
    Device_UART_SendString("  STATUS          Show system status\r\n");
    Device_UART_SendString("==============================\r\n");
    s_success_count++;
}

/**
 * @brief  LED 命令（LED ON / OFF / TOGGLE）
 */
static CmdResult Cmd_HandleLED(const char *param)
{
    if (strcmp(param, "ON") == 0) {
        Module_LED_SetState(LED_ON);
        Cmd_ReplyOK("LED=ON");
        s_success_count++;
        return CMD_OK;
    } else if (strcmp(param, "OFF") == 0) {
        Module_LED_SetState(LED_OFF);
        Cmd_ReplyOK("LED=OFF");
        s_success_count++;
        return CMD_OK;
    } else if (strcmp(param, "TOGGLE") == 0) {
        Module_LED_Toggle();
        if (Module_LED_GetState() == LED_ON) {
            Cmd_ReplyOK("LED=ON");
        } else {
            Cmd_ReplyOK("LED=OFF");
        }
        s_success_count++;
        return CMD_OK;
    } else {
        Cmd_ReplyError("BAD LED STATE");
        s_error_count++;
        return CMD_ERR_BAD_LED;
    }
}

/**
 * @brief  STATUS 命令
 */
static void Cmd_HandleStatus(void)
{
    char buf[64];
    const char *mode_str;
    const char *led_str;

    /* 模式字符串 */
    if (Module_LED_GetMode() == LED_MODE_MANUAL) {
        mode_str = "MANUAL";
    } else {
        mode_str = "BLINK";
    }

    /* LED 状态字符串 */
    led_str = (Module_LED_GetState() == LED_ON) ? "ON" : "OFF";

    /* 组合返回：MODE=xxx LED=xxx COUNT=xxxx */
    snprintf(buf, sizeof(buf), "MODE=%s LED=%s COUNT=%04lu\r\n",
             mode_str, led_str, (unsigned long)Key_GetEventCount());

    Device_UART_SendString("OK ");
    Device_UART_SendString(buf);
    s_success_count++;
}

/* ---- 解析与分发 ---- */

/**
 * @brief  解析并执行一条命令
 * @param  line  命令行（已去除首尾空白、已转大写）
 * @return 命令结果
 */
static CmdResult Cmd_ParseAndExecute(char *line)
{
    char *cmd;
    char *param;

    /* 空命令 */
    if (line == 0 || *line == '\0') {
        Cmd_ReplyError("EMPTY COMMAND");
        s_error_count++;
        return CMD_ERR_EMPTY;
    }

    /* 找到第一个空格，分割命令和参数 */
    cmd = line;
    param = strchr(line, ' ');
    if (param != 0) {
        *param = '\0';
        param++;
        /* 跳过参数前的多个空格 */
        while (*param == ' ') param++;
    }

    /* --- 命令分发 --- */
    if (strcmp(cmd, "HELP") == 0) {
        Cmd_HandleHelp();
        return CMD_OK;
    }
    else if (strcmp(cmd, "LED") == 0) {
        if (param == 0 || *param == '\0') {
            Cmd_ReplyError("BAD LED STATE");
            s_error_count++;
            return CMD_ERR_BAD_LED;
        }
        return Cmd_HandleLED(param);
    }
    else if (strcmp(cmd, "STATUS") == 0) {
        Cmd_HandleStatus();
        return CMD_OK;
    }
    else {
        Cmd_ReplyError("UNKNOWN COMMAND");
        s_error_count++;
        return CMD_ERR_UNKNOWN;
    }
}

/* ---- 公共接口 ---- */

/**
 * @brief  命令模块初始化
 */
void Module_Cmd_Init(void)
{
    s_success_count = 0;
    s_error_count = 0;
    s_last_error = CMD_OK;
}

/**
 * @brief  命令处理主函数（主循环中调用）
 */
void Module_Cmd_Process(void)
{
    char line_buf[64];
    uint16_t len;
    char *trimmed;

    if (!Device_UART_IsLineReady()) {
        return;
    }

    len = Device_UART_ReadLine(line_buf, sizeof(line_buf));

    /* 超长命令（ReadLine 返回 0 且行就绪时可能是超长）
     * 这里通过 line_buf[0] == '\0' 且 len == 0 来判断
     * 更准确的方式是在 device_uart 增加状态查询，此处简化处理 */
    if (len == 0 && line_buf[0] == '\0') {
        /* 可能是空行，也可能是超长行
         * 通过检测空行：如果原始行就是空的，返回空命令错误
         * 超长行在 device_uart 中设置了特殊标记，但此处无法区分
         * 基础题统一返回 EMPTY 即可，或增加接口区分 */
        Cmd_ReplyError("EMPTY COMMAND");
        s_error_count++;
        s_last_error = CMD_ERR_EMPTY;
        return;
    }

    /* 处理：去空格 + 转大写 */
    trimmed = Cmd_Trim(line_buf);
    Cmd_ToUpper(trimmed);

    /* 解析执行 */
    s_last_error = Cmd_ParseAndExecute(trimmed);
}

/**
 * @brief  获取成功计数
 */
uint32_t Module_Cmd_GetSuccessCount(void)
{
    return s_success_count;
}

/**
 * @brief  获取错误计数
 */
uint32_t Module_Cmd_GetErrorCount(void)
{
    return s_error_count;
}

/**
 * @brief  获取最近错误
 */
CmdResult Module_Cmd_GetLastError(void)
{
    return s_last_error;
}
