# 中断 vs 主循环思考指南

> 一段逻辑该放中断还是主循环？核心原则：中断越短越好，能放主循环就放主循环。

---

## 判断决策树

```
这段逻辑...
│
├─ 必须在 us 级立刻响应？ ──────────> 中断（如 PWM 输出、呼吸灯刷新）
│
├─ 只是"事件发生了"的感知？ ───────> 中断只清标志+置位，主循环处理
│
├─ 涉及协议解析/Flash？ ───────────> 主循环（中断里禁止）
│
├─ 只需触发已构造好的 Event？ ─────> 可在中断调用 Trigger（不做 Handle/日志）
│
├─ 需要调用非 ISR 安全的模块 API？ ─> 主循环（中断里不跨模块执行业务逻辑）
│
└─ 计算量大/可能阻塞？ ────────────> 主循环
```

---

## 中断里允许做的（白名单）

- 清中断标志
- 计数 / 置标志位
- 极轻量输出：经过评审并标注 ISR 安全的 `xhh_Task_<X>_Out()`
- 唤醒相关的 GPIO 清标志
- `xhh_Event_Trigger`（仅写入已构造的事件与参数；不做日志或业务调用）

```c
// 正确：中断里只清标志 + 计数 + 已声明的轻量输出
void xhh_BSP_Timer_100us_IRQHandler(void)
{
    if (xhh_BSP_Timer_100us_IT_Get())
    {
        xhh_BSP_Timer_100us_IT_Clear();
        xhh_Task_Motor_Out();                   // 已声明 ISR 安全的单次输出
    }
}
```

## 中断里禁止做的（黑名单）

- ❌ 协议帧解析（在主循环 FIFO 出队后做）
- ❌ `xhh_Event_Handle`（事件分发只能在主循环）
- ❌ Flash 读写（擦写耗时阻塞中断）
- ❌ 复杂状态机推进
- ❌ 长循环 / 大块数据处理
- ❌ 跨模块调 API

---

## 共享数据怎么处理

| 情况 | 做法 |
|------|------|
| 中断写、主循环读 | 变量加 `volatile` |
| 多中断共享资源 | 用运行标志位防重入 |
| 需要原子读多字节 | 关中断后读、读完开中断（按 MCU API） |

```c
// volatile：中断写、主循环读
volatile uint16_t tim_count = 0;

// 重入保护标志
uint8_t xhh_Task_ADC_Run_Flag = 0;   // 主循环置1，中断先判断
```

---

## 周期任务放哪

| 周期 | 放哪 | 怎么调度 |
|------|------|----------|
| us 级（100us） | 硬件 Timer 中断 | 调用已声明 ISR 安全的单次输出或置标志 |
| ms 级（1ms） | 优先 SysTick | SysTick 置 1ms 标志；无法安全复用时才用硬件 Timer |
| 10ms / 100ms / 1s | 主循环 | TMOS 事件位 `MAIN_10MS_EVT` 等 |

TMOS 周期任务自重启模式：

```c
case MAIN_10MS_EVT:
    xhh_Event_Handle();
    xhh_SYS_Handle();
    ...
    tmos_start_task(MAIN_TaskID, MAIN_10MS_EVT);   // 重启周期
    return (events ^ MAIN_10MS_EVT);               // 清除已处理位
```

---

## 常见错误

| 错误 | 后果 | 正确做法 |
|------|------|----------|
| 中断里直接 Handle Event | 业务重入、时序乱 | ISR 只 Trigger，主循环 Handle |
| 中断里写 Flash | 阻塞、丢中断 | 关机/主循环时写 |
| 中断和主循环共享变量没加 volatile | 优化导致主循环读不到新值 | 加 volatile |
| 把 10ms 任务塞进中断 | 中断太长、挤占其他中断 | 10ms 放主循环 TMOS |
| 中断里调其他 Task 的 API | 跨模块耦合、重入风险 | 中断只置标志，主循环调 |
