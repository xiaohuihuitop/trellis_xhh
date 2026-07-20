# 事件系统

> 事件是"协议/按键/硬件 → 状态机/Task"之间的统一桥梁。用全局变量承载 + 主循环轮询分发,不用 RTOS 消息队列或回调链。

---

## 机制结构

| 组成 | 形式 |
|------|------|
| 事件值 | Event 文件私有 `static xhh_event_slot` |
| 事件参数 | Event 文件私有 `static xhh_event_parameter_slot` |
| 触发 | `xhh_Event_Trigger(event, parameter)` 写入单槽 |
| 分发 | `xhh_Event_Handle()` 复制事件和参数快照、清空事件槽后进入 `switch` |

单槽设计:同一时刻只持有一个事件。新事件会覆盖未处理的旧事件。这是有意的(嵌入式场景事件密度低),不要改成队列。

**何时才考虑升级成队列**:确认 10ms 周期内会来 ≥2 个事件且后一个不能丢(如高频协议帧 + 按键并发)。先测再改,多数项目用不上。

---

## 参数编码(必须遵守)

事件参数是 `uint32_t`,**必须按位拆分**:

- **高 16bit**:事件来源 ID
- **低 16bit**:数据(再分高字节 `data_h` / 低字节 `data_l`)

```c
// xhh_Module/xhh_Event/xhh_Event.h
#define xhh_Event_Parameter_ID_NULL  0x00000000
#define xhh_Event_Parameter_ID_BLE   0x11110000
#define xhh_Event_Parameter_ID_Touch 0x22220000
#define xhh_Event_Parameter_ID_Key   0x33330000
```

### Handle 里必须提前完整提取

`xhh_Event_Handle()` 取出事件后,**必须先把来源和数据字节都提取好**,即使这个事件可能用不到数据:

```c
void xhh_Event_Handle(void)
{
    xhh_Event_t event_temp = xhh_event_slot;
    uint32_t param = xhh_event_parameter_slot;
    xhh_event_slot = xhh_Event_Null;             // 取出即清空，后续新事件进入下一轮处理

    if (event_temp == xhh_Event_Null) return;

    // 当前事件需要拆分参数时，从局部快照提取。
    uint16_t data_16 = (uint16_t)(param & 0xffff);
    uint8_t  data_h  = (uint8_t)((data_16 >> 8) & 0xff);
    uint8_t  data_l  = (uint8_t)(data_16 & 0xff);

    XHH_DEBUG("e_h:%d\r\n", event_temp);

    switch (event_temp) {
    case xhh_Event_Init:
        xhh_SYS_Change(xhh_SYS_Wake);
        break;
    ...
    }
}
```

> 参数只存在于本次 Handle 的局部快照中。事件处理期间再触发的新事件会写入单槽，留给下一次 Handle 处理。

---

## 触发与分发

```c
void xhh_Event_Trigger(xhh_Event_t event, uint32_t xhh_Event_Parameter)
{
    xhh_event_slot = event;
    xhh_event_parameter_slot = xhh_Event_Parameter;
    XHH_DEBUG("e_t:%d\r\n", event);
}
```

关键点:
- `Handle` 第一件事是**取出并清空** `xhh_event_slot`，避免重入
- `Handle` 在主循环 10ms 周期调用
- 事件 case 内直接调 `xhh_SYS_Change(...)` + 各 `xhh_Task_*_Set_*(...)` 完成多模块联动

---

## 事件来源汇入

所有外部输入必须通过 `xhh_Event_Trigger` 汇入事件系统:

| 来源 | 触发点 |
|------|--------|
| BLE 协议(有 BLE 的项目) | 协议帧解析后 |
| 按键 | `Key_Handle` 内 |
| 硬件状态 | 状态机/Task 内检测到条件(低电、超时等) |

不要在协议层或按键层直接调状态机/Task,必须走事件。

---

## 事件 case 内的多模块联动

一个事件常需同时改多个维度(mode/ui/motor/timeout)。在事件 case 内集中设置,再切状态机:

```c
// 伪代码:事件 case 内集中设置所有联动模块
case xhh_Event_ON_To_Run:
    xhh_Task_Motor_Set_*(...);
    xhh_Task_LED_Set_*(...);
    xhh_Task_TIMEOUT_Set_*(...);
    xhh_Task_UI_Set_*(...);
    xhh_SYS_Change(xhh_SYS_Run);
    break;
```

这保证多模块联动原子化,不会状态改一半。

---

## 前置守卫

事件 case 内常用 `xhh_IS_*` 谓词做前置合法性检查:

```c
if (xhh_Mode_Get_N() != xhh_Mode_Work) break;   // 模式不对就不处理

if (xhh_IS_Can_KEY()) xhh_Event_Trigger(xhh_Event_PowerON, ...);
```

---

## 禁止

- 引入 RTOS 消息队列 / 回调链替代单槽轮询
- 在协议层 / 按键层直接调 `xhh_SYS_Change` 或 `xhh_Task_*`(必须经事件)
- 事件 case 内改了一半状态就 break(要么全改要么不改)
- `Handle` 里不提取参数直接 switch(必须先提取 `data_16/data_h/data_l`)
- 把事件参数改成结构体指针(单槽 uint32_t 编码是有意约束)
