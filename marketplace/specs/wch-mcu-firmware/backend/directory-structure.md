# 目录结构

> 固件按"协议入口 → 事件 → 状态机 → Task 模块 → BSP 支撑"分层。厂商层不改，只调 config.h。

---

## 目录骨架

```text
APP/                    # 应用层：调度入口、协议收发、Flash 辅助
├── main.c              # 启动入口、主循环
├── main_task.c         # TMOS 任务注册、10ms/100ms/1s 周期调度
├── BLE_SLAVE.c         # BLE 外设传输层
├── BLE_HANDLE.c        # BLE 协议帧解析入口
├── xhh_protocols/      # 协议帧结构 + 解析
├── flash/              # Flash 读写原语封装
├── lcd*                # LCD 驱动（厂商示例残留，允许保留原样）
├── ui/                 # UI 资源（RLE 图片等）
└── utc_time/           # 时间换算

xhh_Module/             # 自研核心业务
├── xhh_Event/          # 事件枚举、Trigger、Handle 统一分发
├── xhh_Mode/           # 系统状态机、模式状态
└── xhh_Task/           # 功能 Task 模块（ADC/BAT/UI/Motor/Flash/Touch...）
    └── xhh_Task_ALL.h  # 聚合头

SRC/                    # 厂商底层：启动、链接脚本、CMSIS、外设驱动（不改）
LIB/                    # 厂商 BLE 协议栈二进制库（不改）
HAL/                    # 厂商 HAL 模板（基本不改，只在 config.h 调宏）
Profile/                # BLE GATT profile（厂商模板风格）
tools/                  # 构建工具、post-build CRC
```

---

## 分层职责

### 协议入口层（APP/）
- 协议帧解析、命令识别、参数拆解
- 把跨模块行为翻译成事件或明确的模块调用
- 不在这里散写完整业务流程

### 事件层（xhh_Module/xhh_Event/）
- 统一协调多模块联动
- 当一个动作同时影响 mode/sys/ui/motor/timeout 多个维度时，优先通过事件层串起来
- 详见 [event-system.md](./event-system.md)

### 状态机层（xhh_Module/xhh_Mode/）
- 系统电源、运行、充电、休眠、唤醒等状态
- step/loop 级状态推进
- 详见 [state-machine-pattern.md](./state-machine-pattern.md)

### Task 模块层（xhh_Module/xhh_Task/）
- 每个硬件/业务域一个 `xhh_Task_*` 模块
- 模块内部维护 static 状态，提供四件套 API
- 详见 [task-module-pattern.md](./task-module-pattern.md)

### BSP 支撑
- 每模块自封 BSP（static 函数），无统一 BSP 层
- 厂商硬件操作在 `SRC/StdPeriphDriver/`，HAL 在 `HAL/`

---

## 厂商层约定

- `SRC/` `LIB/` `HAL/` `Profile/` 是厂商代码，**不修改源文件**
- 只在 `HAL/include/config.h` 里调宏（BLE/SLEEP/缓冲等参数）
- 厂商文件保持原命名、原版权头、原格式（不强制套 xhh_ 约定）
- LCD 驱动（`lcd*.c`）允许保留厂商示例原样，包括 `u8/u16` 类型残留

---

## 命名约定

见 [naming-conventions.md](./naming-conventions.md)。

---

## 初始化后应补充的项目事实

- 实际目录名（若与上述不同）
- 实际模块前缀
- 真实调度入口文件名
- 真实状态机文件名
