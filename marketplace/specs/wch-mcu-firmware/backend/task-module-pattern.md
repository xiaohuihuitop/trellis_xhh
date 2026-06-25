# Task 模块模式

> 每个功能域 = 一个 `xhh_Task_<X>.c/.h` 模块，遵循统一的"四件套"接口。这是本项目最一致的模式，10 个 Task 模块全部遵守。

---

## 四件套接口

每个 Task 模块的头文件固定暴露：

| 函数 | 签名 | 职责 |
|------|------|------|
| `_Init` | `void xhh_Task_<X>_Init(void)` | 硬件/GPIO/定时器配置，注册资源 |
| `_DeInit` | `void xhh_Task_<X>_DeInit(void)` | 反初始化，释放资源 |
| `_Cmd` | `void xhh_Task_<X>_Cmd(uint8_t cmd)` | 使能控制，cmd=0 关 / 1 开 |
| `_Loop` | `void xhh_Task_<X>_Loop(void)` | 主循环周期调用，跑模块逻辑 |

部分模块按需扩展 `_Set_*` / `_Get_*` / `_Apply_*`，但四件套是底线。

示例（真实代码）：

```c
// xhh_Module/xhh_Task/xhh_Task_HoldPP.h:6-10
void xhh_Task_HoldPP_Init(void);
void xhh_Task_HoldPP_DeInit(void);
void xhh_Task_HoldPP_Cmd(uint8_t cmd);
void xhh_Task_HoldPP_Loop(void);
void HoldPP_BSP_Set(uint8_t en);
```

---

## 内部状态封装

- 模块状态用 `static` 变量封装在 `.c` 内，不暴露到头文件
- 使能位统一：`static volatile uint8_t xhh_task_<x>_en = 0;`
  - `volatile`：可能被中断或事件层修改
  - `static`：限制在文件内（历史代码有漏 static 的，新代码必须加）

```c
// xhh_Module/xhh_Task/xhh_Task_Motor.c:4
static volatile uint8_t xhh_task_motor_en = 0;

// xhh_Module/xhh_Task/xhh_Task_Key.c:8-17
static uint8_t key_OK_step = 0;
static uint16_t key_OK_interval = 0;
```

---

## Loop 首句守卫（强制）

`_Loop` 函数第一句必须检查使能位，未使能直接返回：

```c
// xhh_Module/xhh_Task/xhh_Task_HoldPP.c:46-49
void xhh_Task_HoldPP_Loop(void) {
    if (xhh_task_holdpp_en == 0) return;
    ...
}
```

所有 Task 模块（`xhh_Task_LED` / `xhh_Task_BAT` / `xhh_Task_Touch` / `xhh_Task_TIMEOUT` ...）都遵守此守卫。新模块必须延续。

---

## BSP 封装（static）

- 模块内的硬件操作（GPIO/定时器/PWM 细节）封装成 `static` 函数，命名 `<模块>_BSP_Set_<对象>`
- BSP 函数**不进头文件**，在 `.c` 顶部前向声明

```c
// xhh_Module/xhh_Task/xhh_Task_Motor.c:45-48（前向声明）
static void Motor_BSP_Set_EN(uint8_t set);
static void Motor_BSP_Set_Level(uint8_t level);

// xhh_Module/xhh_Task/xhh_Task_Motor.c:99-154（实现，封 GPIO/PWM 细节）
```

即每模块自封 BSP，无统一 BSP 层。不要为追求"统一 BSP 抽象"而把硬件细节抽到公共层。

---

## 聚合层 xhh_Task_ALL

- `xhh_Task_ALL.h` 聚合 include 所有 Task 头，并声明 `xhh_Task_ALL_Init/Cmd/DeInit`
- `xhh_Task.c` 实现里逐个转发调用每个模块的四件套
- 主循环和状态机通过聚合接口统一开关模块，不直接逐个调

```c
// xhh_Module/xhh_Task/xhh_Task_ALL.h:1-19
#include "xhh_Task_ADC.h"
// ... 10 个 Task 头
void xhh_Task_ALL_Cmd(uint8_t en);
void xhh_Task_ALL_Init(void);
void xhh_Task_ALL_DeInit(void);
```

新增 Task 模块时：先建 `xhh_Task_<X>.c/.h`，再在 `xhh_Task_ALL.h` 加 include，在 `xhh_Task.c` 的 Init/Cmd/DeInit 里转发。

---

## 调度方式

- 主循环（TMOS）按 10ms/100ms/1s 周期调各模块 `_Loop`
- 状态机层按需用 `_Cmd(1)/_Cmd(0)` 开关模块

```c
// APP/main_task.c:132-167（10ms 周期）
xhh_Event_Handle();
xhh_SYS_Handle();
xhh_Task_HoldPP_Loop();
Key_Handle(xhh_Task_Key_Scanf());
xhh_Task_Motor_Loop();
xhh_Task_UI_Loop();

// xhh_Module/xhh_Mode/xhh_Mode.c:188-189（状态机内按需开关）
xhh_Task_Key_Cmd(1);
xhh_Task_ADC_Cmd(1);
```

---

## 禁止

- 把模块状态暴露成非 static 全局（除使能位确需跨文件时用 `g_` 前缀）
- 在 `_Loop` 里漏掉使能位守卫
- 新建功能时不建 Task 模块，直接在 `main_task.c` 里堆逻辑
- 把 BSP 细节写进头文件
