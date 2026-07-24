# 异常与边界处理

> 无 assert。过程式错误处理：guard early + 返回码 + 枚举状态 + 事件驱动状态切换。

---

## 错误处理三模式

| 模式 | 适用 | 做法 |
|------|------|------|
| Guard early | 入参/运行时前置条件不满足 | `if (...) return;` 立即退出 |
| 返回码 | 协议/辅助调用 | 返回 `xhh_result_t` 枚举，调用方判 `!= xhh_result_ok` |
| 状态切换 | 用户可感知/硬件级故障 | 映射成事件或 `xhh_SYS_Change(ERR/PowerOFF)` |
| 致命停机 | BSP 固定参数错误、初始化失败、硬件通信超时 | 调用 `xhh_BSP_SYS_ERR_Handle()`，内部 `while (1)` |

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

---

## 占位函数与 fallback 的边界

为保持已经确认的 BSP 生命周期、Task Cmd 聚合和跨项目公开接口统一，允许保留明确登记的占位函数。Task 不再为统一外形保留 `_Init/_DeInit`，但统一 Cmd 可以占位。占位必须显式可审查，不能伪装成真实实现。

```c
void xhh_BSP_Power_Enter_Wake(void)
{
	/* AI:占位接口：当前产品未启用低功耗；确认唤醒后的硬件恢复需求后实现。 */
}

xhh_BSP_Status_t xhh_BSP_Timer_Init_100us(
	xhh_BSP_Timer_Callback_t callback)
{
	(void)callback;
	/* AI:占位接口：当前项目未分配准确的 100us 定时资源，确认资源后实现。 */
	return xhh_BSP_ERROR_UNSUPPORTED;
}
```

- 函数体内必须使用 `AI:` 注释说明占位原因、当前行为和后续启用条件。
- `.h` 中的 Doxygen 必须同步说明当前项目为占位接口。
- `void` 占位函数不得操作硬件；返回状态的占位函数必须返回明确的 `UNSUPPORTED`。
- 直接返回值代表有效硬件数据的接口不得用 0、固定采样值或其他合法值占位；应改为状态码加输出参数，或在实现前禁止调用。
- 显式占位不等于 fallback。fallback 是在失败或不支持时偷偷采用另一套默认行为，仍然禁止，新增前必须由用户确认。

范围校验使用 guard early；非法参数不改变模块状态，也不下发默认硬件值：

```c
if (level < MOTOR_LEVEL_MIN || level > MOTOR_LEVEL_MAX)
{
	return;
}
```

只有用户明确批准的安全关断值才能作为特定状态的正式业务行为，并需要使用命名常量和注释说明；不得把它作为非法参数的通用 fallback。

---

## 外部输入与 Task 参数边界

外部输入和 Task 业务接口的参数可信度不同，必须在边界处明确处理：

- 协议、按键、BLE 等外部输入在解析层或事件参数解析处完成长度、取值范围和枚举合法性校验；不合法的数据不得触发业务 Event，也不得传给 Task。
- `xhh_Task_*` 的 `_Set_*`、`_Apply_*`、`_Cmd` 只接收已验证的内部业务参数。非法枚举值、非法资源参数或不可能出现的内部状态，视为调用链错误，必须在修改 Task 状态或下发硬件前调用 `xhh_BSP_SYS_ERR_Handle()`。
- 用于校验外部数据或 Flash 数据的 `IS_Valid` 类接口是例外：可以返回 `xhh_BSP_ERROR_PARAM`，由调用方决定丢弃数据、恢复默认数据或进入业务错误状态。
- 不允许以“保持旧状态”“截断到 MAX”“映射到默认值”处理非法 Task 参数；这些都会隐藏调用错误或形成未经确认的 fallback。

```c
/* AI:协议参数在触发 Event 前完成范围校验。 */
if (level > XHH_TASK_BAT_WORK_MAX)
{
	return;
}
xhh_Event_Trigger(xhh_Event_BAT_Set_Level, level);

/* AI:Task 只接收可信内部枚举；异常值属于调用链错误。 */
default:
	xhh_BSP_SYS_ERR_Handle();
	return;
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

- BSP 固定参数错误、初始化失败或硬件通信超时 → `xhh_BSP_SYS_ERR_Handle()` 停机
- `xhh_BSP_SYS_Reset()` 只用于明确复位需求，禁止代替错误处理
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

Task 不定义独立的 `xhh_Module_Status_t`。已有 Task 接口确实需要向调用方表达成功或失败时，统一复用 `xhh_BSP_Status_t`：

| 函数类型 | 返回 |
|----------|------|
| Task 操作类（`Cmd` / `Set` / `Apply`） | 通常为 `void`；不为形式统一新增返回值 |
| Task 现有状态接口（采样、数据校验等） | `xhh_BSP_Status_t`，成功为 `xhh_BSP_OK` |
| Task `Loop` | 仅在当前接口确实需要上报状态时使用 `xhh_BSP_Status_t`；原本为 `void` 的 Loop 保持 `void` |
| 查询类 | 业务值或枚举 |
| 谓词 | `uint8_t`，仅表示 0/1 真值 |
| 协议层解析 | 当前项目定义的协议结果枚举 |

- 状态码判断必须使用 `xhh_BSP_OK`、`xhh_BSP_ERROR_PARAM` 等命名枚举，禁止把裸 `0/1` 当作状态码。
- 不为了统一接口形态给无失败语义的 Task 函数增加返回值，也不改变既有 `void` 函数的控制流。

---

## 禁止

- 引入 assert / 异常机制
- 一半忽略、一半改状态（拒绝时在改核心状态前就拒绝）
- 多模块联动不走统一事件，造成状态分叉
- 用 web 风格错误包装层
- 占位函数没有明确的 `AI:` 注释，或返回默认成功、伪造硬件值
- 把占位函数当作已经实现的能力调用

---

## 初始化后应补充的项目事实

- 实际返回码类型全集
- 实际故障事件清单
- 哪些协议边界是"已知接受的假设"（发送端保证输入合法）
