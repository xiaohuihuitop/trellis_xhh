# 目录结构

> 本 spec 只约束 `xhh_Module/` 业务层。平台层目录随 MCU/IDE 变化,不统一约束。

---

## 本 spec 约束的部分:xhh_Module 业务层

所有项目共享的固定结构:

```text
xhh_Module/                     # 自研核心业务(本 spec 约束的全部范围)
├── xhh_Event/                  # 事件枚举、Trigger、Handle 统一分发
│   ├── xhh_Event.h
│   └── xhh_Event.c
├── xhh_Mode/                   # 系统状态机、模式状态
│   ├── xhh_Mode.h
│   └── xhh_Mode.c
└── xhh_Task/                   # 功能 Task 模块
    ├── xhh_Task_ADC.c/.h
    ├── xhh_Task_BAT.c/.h
    ├── xhh_Task_Motor.c/.h
    ├── xhh_Task_Flash.c/.h
    ├── ...                     # 按项目需求增减
    ├── xhh_Task.c             # 聚合实现(转发各模块四件套)
    └── xhh_Task_ALL.h         # 聚合头
```

---

## 分层职责(仅 xhh_Module 内)

### 事件层(xhh_Module/xhh_Event/)
- 统一协调多模块联动
- 当一个动作同时影响 mode/sys/ui/motor/timeout 多个维度时,必须通过事件层串起来
- 详见 [event-system.md](./event-system.md)

### 状态机层(xhh_Module/xhh_Mode/)
- 系统电源、运行、充电、休眠、唤醒等状态
- step/loop 级状态推进
- 详见 [state-machine.md](./state-machine.md)

### Task 模块层(xhh_Module/xhh_Task/)
- 每个硬件/业务域一个 `xhh_Task_*` 模块
- 模块内部维护 static 状态,提供四件套 API
- 详见 [task-module.md](./task-module.md)

### BSP 隔离(平台无关的关键)
- 每模块自封 BSP(static 函数),无统一 BSP 层
- 换 MCU 只改 BSP static 函数实现,模块对外接口不变
- 厂商硬件操作在平台层(具体目录由 MCU 定)

---

## 平台层(不在本 spec 约束范围)

平台层随 MCU/IDE 变化,每次不同,**本 spec 不统一约束**。两种已知形态示例:

### WCH / MounRiver 类

```text
APP/                # 调度入口、协议收发、Flash 辅助
├── main.c
├── main_task.c     # TMOS 调度
├── BLE_SLAVE.c / BLE_HANDLE.c / xhh_protocols/   # BLE(项目有 BLE 时)
├── flash/
└── ...
HAL/  SRC/  LIB/  Profile/  tools/    # 厂商层
```

### PY32 / MDK 类

```text
Inc/                # HAL 接口头
Src/                # main.c、it.c、外设初始化
├── main.c          # 裸 while(1) + tick flag 调度
└── py32f0xx_it.c   # 中断
Lib/Drivers/        # PY32 HAL / CMSIS
MDK_V5/             # Keil 工程
xhh_Module/         # 业务层(本 spec 约束)
ui/                 # RLE 图片资源(按需)
```

平台层规则由项目按 MCU 自填。本 spec 要求:**`xhh_Module/` 不依赖平台目录名**,通过 BSP static 函数隔离硬件。

---

## BLE 部分(有 BLE 的项目适用)

有 BLE 的项目,协议入口链路:

```text
BLE_SLAVE.c         # BLE 外设传输层
BLE_HANDLE.c        # 协议帧解析入口
xhh_protocols/      # 帧结构(head/id/fun/data/crc16/tail)+ 解析
Profile/            # GATT profile
app_drv_fifo/       # 字节缓冲 FIFO
```

协议入口 → 事件层链路:BLE 收数回调 → FIFO 缓冲 → 主循环出队 → `BLE_HANDLE` 解析帧 → `xhh_Event_Trigger`。详见 [event-system.md](./event-system.md) 的事件来源汇入。

无 BLE 的项目没有这层,事件来源是按键/硬件状态,直接 `xhh_Event_Trigger`。

---

## 命名约定

见 [naming-conventions.md](./naming-conventions.md)。

---

## 初始化后应补充的项目事实

- 本项目 MCU 平台类型
- 平台层目录名
- 是否有 BLE(有 BLE 才有协议入口层)
- 真实调度入口文件名