# 目录结构

> MCU 固件项目固定使用五个源码目录 `APP/`、`xhh_Module/`、`xhh_BSP/`、`Platform/`、`Components/`，并使用 `Project/`、`Doc/` 两个工程辅助目录。每个项目只包含一种 MCU 平台，不在同一项目内并存多个芯片平台。

---

## 固定七个顶层目录

```text
项目根目录/
├── APP/                         # 入口与产品集成
├── xhh_Module/                  # Event/Mode/Task 业务逻辑
├── xhh_BSP/                     # 当前 MCU 基础外设驱动
├── Platform/                    # 当前项目唯一 MCU 平台
├── Components/                  # 无厂家依赖的公共组件
├── Project/                     # IDE 工程配置和编译输出
└── Doc/                         # 项目资料
```

前五个目录构成源码分层；`Project/` 和 `Doc/` 不参与源码依赖。`.trellis/` 等工具目录不计入框架目录。

依赖方向固定为：

```text
APP -> xhh_Module -> Components
          |              |
          v              |
       xhh_BSP <----------+
          |
          v
      Platform
```

- `APP` 是组合入口，可完成系统初始化和调度接入，不承载可复用业务模块。
- `xhh_Module` 可依赖 `xhh_BSP` 和 `Components`，禁止依赖 `Platform`。
- `Components` 可依赖 `xhh_BSP` 的公开接口，禁止依赖 `APP`、`xhh_Module` 或 `Platform`。
- `xhh_BSP` 可依赖 `Platform`，禁止反向依赖 `APP`、`xhh_Module` 或 `Components`。
- `Platform` 不依赖任何上层目录。
- `Components` 必须保持产品语义和厂家 SDK 无关；纯算法组件只依赖 C 标准库和自身，LCD 等外部器件组件可通过 `xhh_BSP` 使用基础硬件能力。

---

## APP：入口与产品集成

`APP/` 负责当前产品的程序入口、调度接入、协议入口和产品配置：

```text
APP/
├── main.c                       # 芯片启动后的应用入口
├── main_task.c/.h               # 按需：将 TMOS 或复杂调度从 main.c 拆出
├── BLE_SLAVE.c/.h               # 产品使用 BLE 时按需存在
├── BLE_HANDLE.c/.h              # 产品协议入口
├── config/                      # 产品功能开关
└── assets/                      # UI 图片、字体等产品资源
```

- `main.c` 是固定入口，负责按顺序调用 BSP 初始化、Event/Task 初始触发与主循环调度。调度复杂时，可按需使用 `main_task.c/.h` 拆分调度代码；不建立 APP 硬件 Config 聚合层。
- 协议入口、状态迁移和跨域业务编排必须进入 `xhh_Event`，不得长期堆在 `main.c` 或协议回调中；Task 间的单向局部协作按 Task 规范处理。
- 厂商调度机制只留在 APP 入口适配处，不扩散到 Event/Mode/Task。

---

## xhh_Module：产品业务逻辑

```text
xhh_Module/
├── xhh_Event/                   # 事件枚举、Trigger、Handle
├── xhh_Mode/                    # 系统状态机、模式状态
└── xhh_Task/                    # 各产品功能 Task
```

- `xhh_Event` 统一协调多模块联动。
- `xhh_Mode` 负责系统状态和状态推进。
- `xhh_Task` 负责 Motor、BAT、Key、UI 等产品功能域。
- 业务层禁止 include `main.h`、厂商 SDK 头和芯片寄存器定义。
- `xhh_Task_*` 通过根目录 `xhh_BSP/` 的公开运行期接口访问已初始化硬件，不提供硬件 Config、Init 或 DeInit。

---

## xhh_BSP：当前 MCU 基础外设能力

```text
xhh_BSP/
├── xhh_BSP_Def.h
├── xhh_BSP_GPIO.c/.h
├── xhh_BSP_PWM.c/.h
├── xhh_BSP_IWDG.c/.h
├── xhh_BSP_SPI.c/.h
├── xhh_BSP_ADC.c/.h
├── xhh_BSP_Flash.c/.h
├── xhh_BSP_RTC.c/.h
├── xhh_BSP_Power.c/.h
└── xhh_BSP_SYS.c/.h
```

- xhh_BSP 按 GPIO、PWM、ADC 等基础外设能力拆分；GPIO 公开接口使用端口、引脚和高低电平，不定义项目功能对象；ADC 固定使用 `CHANNEL_0..9` 逻辑槽位。PWM 使用无产品语义的独立操作函数族，例如 `xhh_BSP_PWM1_*`。
- xhh_BSP 实现可调用 `Platform/` 中当前芯片的厂商 API。
- xhh_BSP 公开头禁止泄露厂商类型、厂商宏和寄存器定义。
- 不增加 `Port -> xhh_BSP` 转发层；xhh_BSP 直接将公开参数转换为当前 Platform 的厂商资源。
- BSP 的初始化、运行期操作和资源归属以 [bsp.md](./bsp.md) 的“BSP 生命周期与资源归属”为准。
- 为保持框架接口和生命周期统一，允许保留明确登记的占位函数。占位函数体内必须使用 `AI:` 注释说明占位原因、当前无动作行为和后续启用条件；返回状态的接口必须返回 `xhh_BSP_ERROR_UNSUPPORTED`，禁止默认成功、伪造硬件值或静默 fallback。

详见 [bsp.md](./bsp.md)。

---

## Platform：当前唯一 MCU 平台

`Platform/` 只放当前项目实际使用的一个 MCU 平台，不建立 CH57x、CH59x、PY32、STM32 并存的子目录。该目录保存厂家提供或厂家生成的 SDK、启动、系统、配置和基础中断骨架，不保存项目开发者手写的外设初始化封装。

典型内容：

```text
Platform/
├── Device/                      # CMSIS、RVMSIS、芯片定义
├── Drivers/                     # 厂商 HAL、StdPeriph
├── Startup/                     # 启动文件
├── Linker/                      # 链接脚本、scatter 文件
├── Middleware/                  # 芯片绑定的 BLE、USB 等协议栈
└── Config/                      # 厂商配置头
```

内部目录可根据厂商 SDK 的真实结构裁剪，但所有厂家提供的芯片绑定内容必须收口在 `Platform/`。厂商源码原则上不修改，只调整厂商允许的配置文件和生成文件中的用户配置区。

- 项目手写的 `MX_*` 外设初始化、外设 HAL Handle、DMA 链接、GPIO 复用配置和 MSP 适配应并入对应 `xhh_BSP_*.c`，不得放入 `Platform/Src`。
- 厂家中断骨架可以保留在 `Platform`；只服务于一个 BSP 的 ADC、DMA、Timer 等外设 IRQ 适配应放入对应 `xhh_BSP_*.c`。核心异常、SysTick 和无法归属单一 BSP 的板级中断入口仍可留在厂家中断骨架。
- 当前项目未使用的外设直接不纳入工程，不为保持目录或接口数量一致而新增空 BSP。

---

## Components：公共组件

```text
Components/
├── fifo/
├── protocols/
├── utc_time/
├── rle/
└── lcd/
```

- 仅放跨项目行为稳定、没有产品语义、没有厂家 SDK 或寄存器依赖的组件。
- 纯算法组件只依赖 C 标准库和自身公开头；LCD、传感器等外部器件组件允许 include `xhh_BSP_*.h`，通过已初始化的 GPIO、SPI、I2C、Delay 等基础能力访问硬件。器件连线需要变化时，可通过 BSP 公开的端口、引脚和电平参数适配，但不得配置 MCU 外设。
- Components 禁止感知厂家类型、寄存器、MCU 型号以及 Task/Event/Mode 等产品状态。
- Component 的器件初始化不等于 MCU 外设初始化；Component 不得调用厂家 API 或配置 MCU 外设、GPIO、DMA 和中断。
- 当前项目只使用一次且抽象后更难理解的代码，不进入 `Components/`。
- 同名文件在多个项目中仍有行为差异时，先留在 APP 或 xhh_Module，不为追求复用强行合并。

---

## Project 与 Doc：工程辅助目录

单一 IDE 工程直接放在 `Project/`，不增加无意义的 `MDK/`、`Keil/` 等子目录：

```text
Project/
├── <工程名>.uvprojx
├── <工程名>.uvoptx
├── DebugConfig/
├── Objects/
└── Listings/
```

- `Project/` 只保存 IDE 工程元数据、调试配置和编译输出，不保存本机安装的编译器程序。
- 只有同一源码确实同时维护 Keil、IAR、CMake 等多套工程时，才按工具建立子目录。
- `Doc/` 保存当前项目正式资料，文件名和正文使用中文。

---

## BLE 项目的归属

- 芯片厂商 BLE 库、TMOS 和底层适配放 `Platform/Middleware/`。
- 产品 GATT Profile、协议帧入口和业务命令处理放 `APP/`。
- 纯字节 FIFO、CRC、通用帧解析器以及不含产品语义的外部器件驱动放 `Components/`。

---

## 禁止

- 在一个项目的 `Platform/` 内并存多个 MCU 平台。
- 在 `Platform/Src` 中保存项目手写的 `MX_*`、外设 Handle、DMA 配置或外设初始化封装。
- 单一 IDE 工程仍在 `Project/` 下增加重复工具名子目录。
- 在 `xhh_Module/` 中直接调用厂商 API、寄存器或引脚宏。
- 在 `Components/` 中 include `main.h`、`APP/`、`xhh_Module/`、`Platform/` 或厂商 SDK 头；Components 只能按需依赖 `xhh_BSP/` 公共头。
- 在五个目录之间使用反向依赖或循环依赖。
- 添加没有说明的空函数、弱符号默认实现、默认成功返回值或静默 fallback；为框架统一保留且在函数体内明确注释的占位函数除外。
- 从其他参考项目通过相对路径直接引用源码；每个项目必须可独立编译。

---
