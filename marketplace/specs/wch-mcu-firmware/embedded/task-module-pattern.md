# Task 模块模式

> 每个功能域 = 一个 `xhh_Task_<X>.c/.h` 模块。四件套接口必须完整暴露,内部实现可空。BSP 用 static 封装隔离硬件,平台无关。

---

## 四件套接口(必须完整)

每个 Task 模块的头文件必须暴露:

| 函数 | 签名 | 职责 |
|------|------|------|
| `_Init` | `void xhh_Task_<X>_Init(void)` | 硬件/GPIO/定时器配置,注册资源 |
| `_DeInit` | `void xhh_Task_<X>_DeInit(void)` | 反初始化,释放资源 |
| `_Cmd` | `void xhh_Task_<X>_Cmd(uint8_t cmd)` | 使能控制,cmd=0 关 / 1 开 |
| `_Loop` | `void xhh_Task_<X>_Loop(void)` | 主循环周期调用,跑模块逻辑 |

四件套是底线,必须齐全。**接口内部可以是空实现**(如配置类模块的 `_Loop` 空体 + 守卫即可),但接口必须保留,保证聚合层 `xhh_Task_ALL` 转发时形态一致。

部分模块按需扩展 `_Set_*` / `_Get_*` / `_Apply_*`,但不能替代四件套。

示例:

```c
// xhh_Module/xhh_Task/xhh_Task_HoldPP.h
void xhh_Task_HoldPP_Init(void);
void xhh_Task_HoldPP_DeInit(void);
void xhh_Task_HoldPP_Cmd(uint8_t cmd);
void xhh_Task_HoldPP_Loop(void);
```

---

## 内部状态封装

- 模块状态用 `static` 变量封装在 `.c` 内,不暴露到头文件
- 使能位:`static volatile uint8_t xhh_task_<x>_en = 0;`
  - `volatile`:可能被中断或事件层修改
  - `static`:限制在文件内

```c
static volatile uint8_t xhh_task_motor_en = 0;

static uint8_t key_OK_step = 0;
static uint16_t key_OK_interval = 0;
```

---

## Loop 首句守卫(必须)

`_Loop` 函数第一句必须检查使能位,未使能直接返回:

```c
void xhh_Task_HoldPP_Loop(void) {
    if (xhh_task_holdpp_en == 0) return;
    ...
}
```

即使 Loop 体内是空的,守卫也必须保留。

---

## BSP 封装(static,平台隔离关键)

- 模块内硬件操作(GPIO/定时器/PWM/HAL 调用)封装成 `static` 函数,命名 `<模块>_BSP_Set_<对象>`
- BSP 函数不进头文件,在 `.c` 顶部前向声明
- 这层是 `xhh_Module` 平台无关的关键:换 MCU 只改 BSP static 函数实现,模块对外接口不变

```c
// 顶部前向声明
static void Motor_BSP_Set_EN(uint8_t set);
static void Motor_BSP_Set_Level(uint8_t level);

// 实现:封 GPIO/PWM/HAL 细节(WCH 用 GPIOB_ModeCfg,PY32 用 HAL_GPIO_PinConfig,都封在这)
```

不要为追求"统一 BSP 抽象"把硬件细节抽到公共层——每模块自封 BSP 反而让换平台只动一处。

---

## 聚合层 xhh_Task_ALL

- `xhh_Task_ALL.h` 聚合 include 所有 Task 头,并声明 `xhh_Task_ALL_Init/Cmd/DeInit`
- `xhh_Task.c` 实现里逐个转发调用每个模块的 Init/Cmd/DeInit
- 主循环和状态机通过聚合接口统一开关模块,不直接逐个调

```c
#include "xhh_Task_ADC.h"
// ... 各 Task 头
void xhh_Task_ALL_Cmd(uint8_t en);
void xhh_Task_ALL_Init(void);
void xhh_Task_ALL_DeInit(void);
```

新增 Task 模块时:先建 `xhh_Task_<X>.c/.h`(四件套齐全),再在 `xhh_Task_ALL.h` 加 include,在 `xhh_Task.c` 的 Init/Cmd/DeInit 里转发。

---

## 调度

- 主循环按周期调各模块 `_Loop`(周期机制 TMOS/tick flag 由平台定,见 [interrupt-and-critical-code.md](./interrupt-and-critical-code.md))
- 状态机层按需用 `_Cmd(1)/_Cmd(0)` 开关模块

```c
// 主循环 10ms 周期(平台无关的调用顺序)
xhh_Event_Handle();
xhh_SYS_Handle();
xhh_Task_HoldPP_Loop();
Key_Handle(xhh_Task_Key_Scanf());
xhh_Task_Motor_Loop();
xhh_Task_UI_Loop();

// 状态机内按需开关
xhh_Task_Key_Cmd(1);
xhh_Task_ADC_Cmd(1);
```

---

## 禁止

- 把模块状态暴露成非 static 全局(除使能位确需跨文件时用 `g_` 前缀)
- 在 `_Loop` 里漏掉使能位守卫
- 四件套缺接口(即使实现为空,接口也必须暴露)
- 新建功能时不建 Task 模块,直接在 main 里堆逻辑
- 把 BSP 细节写进头文件