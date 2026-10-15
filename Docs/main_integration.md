# main.c 集成指南

本文档说明如何将业务代码集成到 CubeMX 生成的 `main.c` 中。

## 修改原则

- CubeMX 生成的代码放在 `/* USER CODE BEGIN ... */` 和 `/* USER CODE END ... */` 之间
- 不要修改 CubeMX 生成的代码区域
- main.c 只保留初始化和调用 App 层入口，不堆积业务逻辑

## 具体修改

### 1. 引入头文件

在 `main.c` 顶部的 `/* USER CODE BEGIN Includes */` 区域添加：

```c
/* USER CODE BEGIN Includes */
#include "app.h"
/* USER CODE END Includes */
```

### 2. 串口接收中断回调

在 `main.c` 中 `/* USER CODE BEGIN 0 */` 区域添加（或新建 stm32f1xx_it.c 中添加）：

```c
/* USER CODE BEGIN 0 */
#include "device_uart.h"

/* 串口接收完成中断回调 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Device_UART_RxCpltCallback();
    }
}
/* USER CODE END 0 */
```

> ⚠️ 如果你的工程中 `stm32f1xx_it.c` 里也有这个回调函数，只需保留一份即可，建议放在 `main.c` 中。

### 3. 应用初始化

在 `main()` 函数中，`MX_xxx_Init()` 全部完成之后、`while(1)` 之前的 `/* USER CODE BEGIN 2 */` 区域添加：

```c
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* 应用层初始化：BSP → Device → Component → Module → Task → App */
  App_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* 应用主循环：任务调度器 */
    App_Loop();

    /* USER CODE END 3 */
  }
```

### 4. 完整 main.c 对照

修改后的 `main.c` 大致结构：

```c
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "app.h"           /* ← 添加应用层头文件 */
/* USER CODE END Includes */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN 0 */
#include "device_uart.h"   /* ← 添加设备层头文件（用于回调） */

/**
 * @brief  串口接收完成回调
 * @note   每收到 1 字节调用一次，委托给 Device_UART 处理
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Device_UART_RxCpltCallback();
    }
}
/* USER CODE END 0 */

int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  App_Init();            /* ← 应用初始化（只调用一次） */
  /* USER CODE END 2 */

  /* Infinite loop */
  while (1)
  {
    /* USER CODE BEGIN 3 */
    App_Loop();          /* ← 应用主循环（反复调用） */
    /* USER CODE END 3 */
  }
}
```

## Keil 工程中添加文件和路径

### 添加源文件

1. 打开 Keil 工程
2. 右键点击 Target 1 → **Manage Project Items...**
3. 在 Groups 列表中添加以下分组：
   - `BSP`
   - `Device`
   - `Component`
   - `Module`
   - `Task`
   - `App`
4. 选中每个分组，点击右侧 Add Files，将对应目录下的 `.c` 文件添加进去：

| 分组 | 文件 |
| --- | --- |
| BSP | `BSP/bsp_led.c`、`BSP/bsp_key.c`、`BSP/bsp_uart.c` |
| Device | `Device/device_oled.c`、`Device/device_uart.c` |
| Component | `Component/component_key.c` |
| Module | `Module/module_led.c`、`Module/module_cmd.c`、`Module/module_display.c` |
| Task | `Task/task_scheduler.c` |
| App | `App/app.c` |

### 添加头文件路径

1. 点击魔术棒图标（Options for Target）
2. 切换到 **C/C++** 选项卡
3. 在 **Include Paths** 中点击 `...` 按钮
4. 添加以下路径（相对路径）：
   ```
   ./BSP
   ./Device
   ./Component
   ./Module
   ./Task
   ./App
   ```
5. 点击 OK 保存

## 编译与下载

1. 点击 **Rebuild**（或按 F7）编译整个工程
2. 检查是否有错误
3. 连接 DAPLink，点击 **Download**（或按 F8）下载程序
4. 复位开发板，观察 OLED 和 LED 是否正常

## 常见问题

### 编译报错找不到 `main.h`

`bsp_*.c` 中包含了 `main.h`，这是 CubeMX 生成的总头文件，包含所有外设初始化声明。请确认 Include Paths 中包含 `./Core/Inc`（通常默认已包含）。

### 编译报错 `LED_Pin` / `KEY1_Pin` 未定义

这些宏在 CubeMX 生成的 `main.h` 中定义，对应你在 CubeMX 里给引脚设置的 User Label。请确认：
1. CubeMX 中对应引脚的 User Label 是否正确设置为 `LED`、`KEY1`、`KEY2`、`KEY3`
2. 重新生成代码后再编译

### 串口接收中断不工作

确认：
1. CubeMX 的 USART1 NVIC Settings 中勾选了 "USART1 global interrupt"
2. `HAL_UART_RxCpltCallback` 函数被正确调用（在回调中打个断点测试）
3. `Device_UART_Init()` 被调用（它会开启第一次接收中断）

### OLED 不亮或显示乱码

确认：
1. I2C 地址是否正确（常见 `0x3C`，也有 `0x3D` 的模块）
2. SCL 和 SDA 是否接反
3. I2C 时钟配置是否正确
4. 共地是否良好
