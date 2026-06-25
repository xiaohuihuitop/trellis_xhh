# 中断与关键码

> 中断处理函数和关键性能函数应放在 RAM 中执行以提速。具体段名/属性语法依 MCU 而定，核心思想是"让高频/时敏代码跑在 RAM 而非 Flash"。

---

## RAM 执行原则

- 中断处理函数和时敏关键函数放 RAM 执行段
- WCH RISC-V 上用：`__attribute__((interrupt("WCH-Interrupt-fast")))` + `__attribute__((section(".highcode")))`
- 其他 MCU 用对应语法（如 `__attribute__((section(".ramfunc")))`、`IRAM_ATTR` 等）
- 模板里只写原则，具体属性宏在项目 init 时按 MCU 填入

```c
// APP/main_task.c:291-313（WCH CH573 示例）
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void TMR0_IRQHandler(void)
{
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
        ...
    }
}
```

---

## 中断里只做什么

中断函数必须**极轻量**，只做：

- 清中断标志
- 计数 / 置标志位
- 极轻量输出（LED 呼吸、Motor PWM 输出、HoldPP）

```c
// APP/main_task.c:291-313
TMR0_ClearITFlag(TMR0_3_IT_CYC_END);   // 清标志
tim_count++;                           // 计数
        xhh_Task_LED_Loop();                   // 100us 跑 led
if (tim_count >= 10) {                 // 1ms
    tim_count = 0;
    xhh_Task_Motor_Out();              // 1ms 跑 motor pwm
}
```

---

## 中断里禁止做什么

- ❌ 协议解析（帧解析在主循环出队后做）
- ❌ 事件触发（`xhh_Event_Trigger` 在主循环/事件层调）
- ❌ Flash 读写（擦写耗时且可能阻塞中断）
- ❌ 复杂状态机推进
- ❌ 长循环 / 大块数据处理

---

## 中断与主循环的数据共享

- 共享变量加 `volatile`

```c
// APP/main_task.c:17
volatile uint16_t tim_count = 0;          // 中断写、主循环读

// xhh_Module/xhh_Mode/xhh_Mode.c:28
volatile uint8_t motor_start_flag = 0;
```

- 重入保护用运行标志

```c
// xhh_Module/xhh_Task/xhh_Task_ADC.c:22
uint8_t xhh_Task_ADC_Run_Flag = 0;   // 主循环采集前置1、结束置0，中断先判断避免重入
// 注释：中断中需要先判断才使用adc.否则冲突,可能死循环
```

- GPIO 唤醒中断只清标志，不做任何业务：

```c
// APP/main_task.c:315-328
void GPIOB_IRQHandler(void) { GPIOB_ClearITFlagBit(GPIO_Pin_12); }
```

---

## 主循环调度（TMOS，WCH 特有）

> TMOS 是 WCH BLE 协议栈自带的事件调度机制。其他 MCU 平台用对应的 RTOS 或裸 `while(1)` 轮询；本节调度思路（周期事件位 + 自重启）通用，但 API 名不通用。

本项目主循环走厂商 TMOS 事件调度，不是裸 `while(1)` 轮询：

```c
// APP/main.c:38-44
__attribute__((section(".highcode"))) __attribute__((noinline))
void Main_Circulation() {
    while(1) { TMOS_SystemProcess(); }
}
```

事件位定义与周期任务：

```c
// APP/main_task.h:15-27
#define MAIN_START_EVT  (0x0001<<0)
#define MAIN_10MS_EVT   (0x0001<<2)
#define MAIN_100MS_EVT  (0x0001<<3)
#define MAIN_1S_EVT     (0x0001<<4)
```

**事件返回约定**：处理完一位事件用异或清除该位；周期任务在 case 末尾再次 `tmos_start_task` 自重启：

```c
// APP/main_task.c:165-166
tmos_start_task(MAIN_TaskID, MAIN_10MS_EVT);   // 重启周期
return (events ^ MAIN_10MS_EVT);               // 清除已处理位
```

末尾 `return 0;` 表示无未处理事件（`main_task.c:190`）。

---

## 中断优先级与临界区

- 临界区保护按 MCU 提供 API（WCH 用 `sys_safe_access_enable` / 关中断等）
- 中断优先级配置在初始化阶段一次性设好，运行时不动态改
- 多中断共享资源时，用标志位延迟到主循环处理，而不是在中断里嵌套处理

---

## 初始化后应补充的项目事实

- 本项目 MCU 的 RAM 执行段属性宏
- 实际使用的中断向量清单与优先级
- 临界区保护 API
- TMOS 事件位清单与周期
