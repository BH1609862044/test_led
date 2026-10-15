# 分层设计与数据流向

## 设计目标

采用自底向上的分层架构，每层只依赖其下方的层，业务逻辑与硬件完全解耦。

## 分层架构

```
┌─────────────────────────────────────┐
│              App 层                 │  应用入口、状态编排、事件分发
│          app.h / app.c              │
├─────────────────────────────────────┤
│             Task 层                 │  周期调度、任务管理
│     task_scheduler.h / .c           │
├─────────────────────────────────────┤
│            Module 层                │  业务逻辑：LED 模式、命令解析、显示
│  module_led  module_cmd             │
│  module_display                     │
├─────────────────────────────────────┤
│      Component / Device 层          │  通用组件 + 设备驱动
│  component_key    device_oled       │
│                   device_uart       │
├─────────────────────────────────────┤
│             BSP 层                  │  板级封装：LED、按键、串口
│  bsp_led  bsp_key  bsp_uart         │
├─────────────────────────────────────┤
│         HAL 底层 (CubeMX)           │  HAL 库 + 寄存器
│     Core/  (CubeMX 生成)            │
└─────────────────────────────────────┘
```

依赖方向：**自上而下**，禁止反向调用。

## 各层职责

### 1. BSP 层 (Board Support Package)

**职责**：直接操作硬件外设，向上层提供统一的板级接口。

**规则**：
- 只封装硬件操作，不含任何业务逻辑
- 统一处理有效电平（如 LED 低电平点亮）
- 向上层屏蔽硬件差异（如不同引脚、不同有效电平）

**文件**：
- `bsp_led.h/.c` —— LED 开关、翻转、状态读取
- `bsp_key.h/.c` —— 按键 GPIO 电平读取
- `bsp_uart.h/.c` —— 串口数据发送

### 2. Device 层 (Device Driver)

**职责**：设备级驱动，实现具体外设芯片的操作协议。

**规则**：
- 依赖 BSP 层或 HAL 接口，不直接写寄存器
- 不含业务逻辑（如菜单、模式判断）
- 中断中只做数据搬运，不做协议解析

**文件**：
- `device_oled.h/.c` —— SSD1306 OLED 驱动（初始化、字符显示）
- `device_uart.h/.c` —— 串口按行接收（中断 + 行缓冲）

### 3. Component 层 (Component)

**职责**：通用组件，与具体硬件无关，可复用到其他项目。

**规则**：
- 不依赖具体硬件，通过 BSP/Device 层的接口访问硬件
- 纯算法/逻辑组件
- 可配置参数（如消抖时间）

**文件**：
- `component_key.h/.c` —— 按键消抖组件（状态机式）

### 4. Module 层 (Module)

**职责**：业务逻辑模块，实现具体功能。

**规则**：
- 依赖 Component/Device 层，不直接操作硬件
- 各模块之间尽量独立，通过接口交互
- 持有业务状态（如 LED 模式、命令计数）

**文件**：
- `module_led.h/.c` —— LED 模式控制（手动/闪烁）
- `module_cmd.h/.c` —— 串口命令解析与执行
- `module_display.h/.c` —— OLED 界面组织与刷新

### 5. Task 层 (Task)

**职责**：任务调度与周期执行。

**规则**：
- 不包含具体业务逻辑，只负责任务调度
- 每个任务有独立的执行周期
- 协作式调度（非抢占式）

**文件**：
- `task_scheduler.h/.c` —— 简易协作式调度器

### 6. App 层 (Application)

**职责**：应用入口，全局状态管理，事件分发。

**规则**：
- 编排各层初始化顺序
- 处理全局事件（如按键事件分发）
- 实现启动流程和上电自检
- main.c 只调用 App_Init() 和 App_Loop()

**文件**：
- `app.h/.c` —— 应用入口

## 数据流向

### 按键输入数据流

```
按键按下
   ↓
BSP_Key_ReadLevel()         读取 GPIO 电平
   ↓
Key_Scan()                  消抖状态机
   ↓
App_OnKeyEvent()            事件分发（回调）
   ↓
Module_LED_SetMode() /      执行业务操作
Module_LED_Toggle()
   ↓
Module_Display_RefreshMain() 更新 OLED 显示
   ↓
BSP_LED_Set()               改变 LED 输出
```

### 串口命令数据流

```
串口收到字节（中断）
   ↓
Device_UART_RxCpltCallback()  写入行缓冲，遇 \n 置就绪标志
   ↓
Module_Cmd_Process()          主循环轮询，读取行并解析
   ↓
Module_LED_SetState() 等      执行业务操作
   ↓
Device_UART_SendString()      串口返回结果
   ↓
Module_Display_RefreshMain()  更新 OLED 显示
```

### 显示刷新数据流

```
Task 调度器（100ms 周期）
   ↓
Module_Display_Process()      检查数据变化
   ↓
OLED_ShowString() / OLED_ShowNum()  写入 OLED 显存
   ↓
HAL_I2C_Master_Transmit()     I2C 输出到屏幕
```

## 禁止事项

1. ❌ 业务代码中直接操作 GPIO、UART、I2C 等外设寄存器
2. ❌ OLED 驱动直接调用菜单业务代码
3. ❌ BSP 层包含菜单、参数等业务逻辑
4. ❌ main.c 中堆积业务代码
5. ❌ 一个 .c 文件包含全部功能
6. ❌ 中断回调中执行复杂协议解析或 OLED 刷屏
7. ❌ 使用大量阻塞式 HAL_Delay 代替状态机和周期调度
