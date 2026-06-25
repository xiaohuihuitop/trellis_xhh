# 事件系统

> 事件是"协议/按键/硬件 → 状态机/Task"之间的统一桥梁。用全局变量承载 + 主循环轮询分发，不用 RTOS 消息队列或回调链。

---

## 机制结构

| 组成 | 形式 | 位置 |
|------|------|------|
| 事件值 | 全局变量 `xhh_Event_t Event_n` | `xhh_Event.c:10` |
| 事件参数 | 全局变量 `uint32_t xhh_Event_Parameter_n` | `xhh_Event.c:11` |
| 触发 | `xhh_Event_Trigger(event, parameter)` 写入两个全局量 | `xhh_Event.c:14` |
| 分发 | `xhh_Event_Handle()` 读后清零 + switch | `xhh_Event.c:21` |

单槽设计：同一时刻只持有一个事件。新事件会覆盖未处理的旧事件。这是有意为之（嵌入式场景事件密度低），不要改成队列。

---

## 参数编码

事件参数是 `uint32_t`，按位拆分：

- **高 16bit**：事件来源 ID
- **低 16bit**：数据

```c
// xhh_Module/xhh_Event/xhh_Event.h:73-77
#define xhh_Event_Parameter_ID_NULL  0x00000000
#define xhh_Event_Parameter_ID_BLE   0x11110000
#define xhh_Event_Parameter_ID_Touch 0x22220000
```

处理时拆字节：

```c
// xhh_Module/xhh_Event/xhh_Event.c:30-33
data_16 = (xhh_Event_Parameter_n) & 0xffff;
data_h  = (data_16 >> 8) & 0xff;
data_l  =  data_16        & 0xff;
```

---

## 触发与分发

```c
// xhh_Module/xhh_Event/xhh_Event.c:14-35
void xhh_Event_Trigger(xhh_Event_t event, uint32_t xhh_Event_Parameter)
{
    Event_n = event;
    xhh_Event_Parameter_n = xhh_Event_Parameter;
}

void xhh_Event_Handle(void)
{
    xhh_Event_t event_temp = Event_n;
    Event_n = xhh_Event_Null;           // 取出即清零
    ...
    switch (event_temp) {
        case xhh_Event_Init:
            xhh_SYS_Change(xhh_SYS_Wake);
            ...
```

关键点：
- `Handle` 第一件事是**取出并清零** `Event_n`，避免重入
- `Handle` 在主循环 10ms 周期调用（`APP/main_task.c:132`）
- 事件 case 内直接调 `xhh_SYS_Change(...)` + 各 `xhh_Task_*_Set_*(...)` 完成多模块联动

---

## 事件来源汇入

所有外部输入都统一通过 `xhh_Event_Trigger` 汇入事件系统：

| 来源 | 触发点 | 示例 |
|------|--------|------|
| BLE 协议 | `BLE_HANDLE.c:681` 帧解析后 | `xhh_Event_Trigger(xhh_Event_PowerON, ...)` |
| 按键 | `xhh_Task_Key.c:422-567` `Key_Handle` 内 | `xhh_Event_Trigger(xhh_Event_Change_Mode, ...)` |
| 硬件状态 | 状态机/Task 内检测到条件 | 低电、超时等 |

不要在协议层或按键层直接调状态机/Task，统一走事件。

---

## 事件 case 内的多模块联动

一个事件常需同时改多个维度（mode/ui/motor/timeout）。在事件 case 内集中设置，再切状态机：

```c
// 伪代码：事件 case 内集中设置所有联动模块（参考 xhh_Event.c 的 ON_To_Run 分支）
case xhh_Event_ON_To_Run:
    xhh_Task_Motor_Set_*(...);
    xhh_Task_LED_Set_*(...);
    xhh_Task_TIMEOUT_Set_*(...);
    xhh_Task_UI_Set_*(...);
    xhh_SYS_Change(xhh_SYS_Run);
    break;
```

这保证多模块联动原子化，不会状态改一半。

---

## 前置守卫

事件 case 内常用 `xhh_IS_*` 谓词做前置合法性检查：

```c
// xhh_Module/xhh_Event/xhh_Event.c:264-265
if (xhh_Mode_Get_N() != xhh_Mode_Work) break;   // 模式不对就不处理

// APP/BLE_HANDLE.c:681-683
if (xhh_IS_Can_KEY()) xhh_Event_Trigger(xhh_Event_PowerON, ...);
```

---

## 禁止

- 引入 RTOS 消息队列 / 回调链替代全局变量轮询
- 在协议层 / 按键层直接调 `xhh_SYS_Change` 或 `xhh_Task_*`（必须经事件）
- 事件 case 内改了一半状态就 break（要么全改要么不改）
- 把事件参数改成结构体指针（单槽 uint32_t 编码是有意约束）

---

## 初始化后应补充的项目事实

- 实际事件枚举全集（本项目约 40 个事件）
- 实际事件来源 ID 清单
- 哪些事件需要前置守卫、守卫条件是什么
