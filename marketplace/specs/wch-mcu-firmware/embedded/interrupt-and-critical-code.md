# 中断与关键码

> 本 spec 约束"业务代码怎么对待中断"的跨平台原则。具体中断属性语法、调度机制、临界区 API 依 MCU/平台,本 spec 不写死。

---

## 中断默认短小(原则)

中断函数默认只做:清标志 / 计数 / 置标志位。

业务逻辑(事件处理、状态机推进、Flash 操作)必须在主循环跑,不进中断。

---

## 两类允许的例外

### 例外 1:时敏输出(允许)

us~ms 级必须立即响应的输出,可以在中断里直接调对应输出函数:

- Motor PWM 输出(`xhh_Task_Motor_Out()`)
- LED 呼吸刷新(`xhh_Task_LED_Loop()`)
- LCD 刷新

理由:主循环 10ms 周期不够快,这类输出天然属于中断职责。

### 例外 2:安全关断(允许部分)

EXTI 检测到危险(短路/过流/紧急停机)时:
- ✅ 中断里**可以立即关硬件 GPIO**(防止硬件继续损坏)
- ✅ 中断里**可以 `xhh_Event_Trigger(xhh_Event_ERR, ...)` 产生事件**
- ❌ 中断里**不可 `xhh_Event_Handle()` 处理事件**——Handle 必须在主循环跑

即:中断可以"先关硬件 + 把 ERR 事件挂上",但事件的处理(切状态机、关各 Task 模块)留给主循环下一个 10ms 周期。

---

## 中断里禁止(不论平台)

- ❌ Flash 擦写(耗时阻塞中断)
- ❌ 协议帧解析(在主循环出队后做)
- ❌ `xhh_Event_Handle()`(事件分发必须在主循环)
- ❌ 大块数据处理 / 长循环
- ❌ 复杂状态机推进(`xhh_SYS_Handle` 在主循环)

---

## 中断/主循环共享数据

- 共享变量加 `volatile`:

```c
volatile uint16_t tim_count = 0;       // 中断写、主循环读
volatile uint8_t motor_start_flag = 0;
```

- 重入保护用运行标志:

```c
uint8_t xhh_Task_ADC_Run_Flag = 0;    // 主循环采集前置1、结束置0,中断先判断避免重入
```

- 多中断共享资源:优先用标志位延迟到主循环处理,不在中断里嵌套处理。

---

## RAM 执行(不约束)

中断函数和时敏关键函数是否放 RAM 执行段,由项目按 MCU/中断频率/Flash 速度自定。本 spec 不强制。

参考:WCH RISC-V 常用 `__attribute__((section(".highcode")))`,PY32 可不强制。具体语法按平台。

---

## 主循环调度(平台自定,本 spec 不约束)

主循环调度方式随平台不同:

### WCH/TMOS

```c
while(1) { TMOS_SystemProcess(); }   // TMOS 事件位 + tmos_start_task 自重启
// 事件返回: return (events ^ MAIN_10MS_EVT);
```

### PY32/裸机

```c
while(1) {
    if (systick_flag_10ms) { systick_flag_10ms = 0; ... }   // tick flag 轮询
}
// SysTick 中断只置 flag,主循环消费
```

两种平台都遵循同一原则:**主循环里调 `xhh_Event_Handle()` + `xhh_SYS_Handle()` + 各 `xhh_Task_*_Loop()`**。具体调度机制由平台定。

---

## 初始化后应补充的项目事实

- 本项目 MCU 的中断向量清单与优先级
- 临界区保护 API(具体函数名)
- 主循环调度机制(TMOS / tick flag / 其他)
- 是否启用 RAM 执行段