# 事件系统

> 事件是"协议/按键/硬件 → 状态机/Task"之间的统一桥梁。用全局变量承载 + 主循环轮询分发,不用 RTOS 消息队列或回调链。

---

## 机制结构

| 组成 | 形式 |
|------|------|
| 事件值 | 全局变量 `xhh_Event_t Event_n` |
| 事件参数 | 全局变量 `uint32_t xhh_Event_Parameter_n` |
| 触发 | `xhh_Event_Trigger(event, parameter)` 写入两个全局量 |
| 分发 | `xhh_Event_Handle()` 读后清零 + 提取参数 + switch |

单槽设计:同一时刻只持有一个事件。新事件会覆盖未处理的旧事件。这是有意的(嵌入式场景事件密度低),不要改成队列。

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
    xhh_Event_t event_temp = Event_n;
    Event_n = xhh_Event_Null;                    // 取出即清零
    xhh_Event_Parameter_t param = xhh_Event_Parameter_n;
    xhh_Event_Parameter_n = xhh_Event_Parameter_ID_NULL;

    if (event_temp == xhh_Event_Null) return;

    // 必须提前完整提取(即使本次用不到也要提)
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

> 即使调用方只传 `xhh_Event_Parameter_ID_NULL`,Handle 里也必须走完整的提取流程。这是规矩,不是按需优化。

---

## 触发与分发

```c
void xhh_Event_Trigger(xhh_Event_t event, uint32_t xhh_Event_Parameter)
{
    Event_n = event;
    xhh_Event_Parameter_n = xhh_Event_Parameter;
    XHH_DEBUG("e_t:%d\r\n", event);
}
```

关键点:
- `Handle` 第一件事是**取出并清零** `Event_n`,避免重入
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

- 引入 RTOS 消息队列 / 回调链替代全局变量轮询
- 在协议层 / 按键层直接调 `xhh_SYS_Change` 或 `xhh_Task_*`(必须经事件)
- 事件 case 内改了一半状态就 break(要么全改要么不改)
- `Handle` 里不提取参数直接 switch(必须先提取 `data_16/data_h/data_l`)
- 把事件参数改成结构体指针(单槽 uint32_t 编码是有意约束)