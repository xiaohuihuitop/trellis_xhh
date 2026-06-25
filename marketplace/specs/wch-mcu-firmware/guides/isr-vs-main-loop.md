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
├─ 涉及协议解析/事件触发/Flash？ ──> 主循环（中断里禁止）
│
├─ 需要调用其他模块的 API？ ───────> 主循环（中断里不要跨模块调）
│
└─ 计算量大/可能阻塞？ ────────────> 主循环
```

---

## 中断里允许做的（白名单）

- 清中断标志
- 计数 / 置标志位
- 极轻量输出：LED 呼吸刷新、Motor PWM 输出、HoldPP 脉冲
- 唤醒相关的 GPIO 清标志

```c
// 正确：中断里只清标志 + 计数 + 轻量输出
void TMR0_IRQHandler(void)
{
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);   // 清标志
        tim_count++;                            // 计数
        xhh_Task_LED_Loop();                    // 100us led
        if (tim_count >= 10)                    // 1ms
        {
            tim_count = 0;
            xhh_Task_Motor_Out();               // 1ms motor pwm
        }
    }
}
```

## 中断里禁止做的（黑名单）

- ❌ 协议帧解析（在主循环 FIFO 出队后做）
- ❌ `xhh_Event_Trigger`（在主循环/事件层调）
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
| us 级（100us） | 中断 | TMR0 中断内直接调 |
| ms 级（1ms） | 中断内分频 | `tim_count >= 10` 分频出 1ms |
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
| 中断里调 `xhh_Event_Trigger` | 事件被覆盖、时序乱 | 中断置标志，主循环触发事件 |
| 中断里写 Flash | 阻塞、丢中断 | 关机/主循环时写 |
| 中断和主循环共享变量没加 volatile | 优化导致主循环读不到新值 | 加 volatile |
| 把 10ms 任务塞进中断 | 中断太长、挤占其他中断 | 10ms 放主循环 TMOS |
| 中断里调其他 Task 的 API | 跨模块耦合、重入风险 | 中断只置标志，主循环调 |
