# 异常与边界处理

> 无 assert。过程式错误处理：guard early + 返回码 + 枚举状态 + 事件驱动状态切换。

---

## 错误处理三模式

| 模式 | 适用 | 做法 |
|------|------|------|
| Guard early | 入参/运行时前置条件不满足 | `if (...) return;` 立即退出 |
| 返回码 | 协议/辅助调用 | 返回 `xhh_result_t` 枚举，调用方判 `!= xhh_result_ok` |
| 状态切换 | 用户可感知/硬件级故障 | 映射成事件或 `xhh_SYS_Change(ERR/PowerOFF)` |

**不使用 assert**（全项目 0 处）。不引入 web 风格异常封装 / 统一错误对象树。

---

## 协议层返回码

```c
// APP/xhh_protocols/protocols.h:35-41
typedef enum {
    xhh_result_ok = 0,
    xhh_result_frame_style,
    xhh_result_frame_len,
    xhh_result_data_len,
    xhh_result_crc16,
} xhh_result_t;
```

调用方判错即丢弃：

```c
// APP/BLE_HANDLE.c:617-620
ret = xhh_protocols_arr_to_frame(pdata, len, &ble_rx_frame);
if (ret != xhh_result_ok) { return; }
```

---

## Guard early

条件不满足立即 return，不嵌套深层：

```c
// xhh_Module/xhh_Task/xhh_Task_Motor.c:450-458
void xhh_Task_Motor_Set_Obj(Motor_Obj_t *obj) {
    if (obj == NULL) { return; }
    Motor = *obj;
}

// xhh_Module/xhh_Task/xhh_Task_Flash.c:83-84
if (data == NULL) return;
```

范围校验用 if + 兜底默认：

```c
// xhh_Module/xhh_Task/xhh_Task_Motor.c:116-124
if (level < MOTOR_LEVEL_MIN || level > MOTOR_LEVEL_MAX) { duty = 400; }
```

---

## 状态合法性前置守卫

用 `xhh_IS_*` 谓词在动作前检查状态合法性：

```c
// xhh_Module/xhh_Event/xhh_Event.c:264-265
if (xhh_Mode_Get_N() != xhh_Mode_Work) break;   // 模式不对就不处理

// APP/BLE_HANDLE.c:681-683
if (xhh_IS_Can_KEY()) xhh_Event_Trigger(xhh_Event_PowerON, ...);
```

---

## 致命错误

- BLE 库初始化失败 → `while(1)` 死锁 + 打印
- 业务错误 → 触发 `xhh_Event_ERR`，事件层统一关模块 + 切 ERR 状态

```c
// xhh_Module/xhh_Event/xhh_Event.c:502-509
case xhh_Event_ERR:
    xhh_Task_UI_Set_ERR();
    xhh_Task_BAT_Cmd(0); xhh_Task_Key_Cmd(0); xhh_Task_Motor_Cmd(0);
    xhh_SYS_Change(xhh_SYS_ERR);
    break;
```

---

## 返回值约定

| 函数类型 | 返回 |
|----------|------|
| 操作类（Init/Cmd/Set） | `void` |
| 查询类 | 值或枚举 |
| 谓词 | `uint8_t` 0/1 |
| Loop 类 | `uint8_t`（0=正常，1=失败），见 `xhh_Task_ADC.c:108-110` |
| TMOS 事件处理 | `events ^ EVT` |

---

## 禁止

- 引入 assert / 异常机制
- 一半忽略、一半改状态（拒绝时在改核心状态前就拒绝）
- 多模块联动不走统一事件，造成状态分叉
- 用 web 风格错误包装层

---

## 初始化后应补充的项目事实

- 实际返回码类型全集
- 实际故障事件清单
- 哪些协议边界是"已知接受的假设"（发送端保证输入合法）
