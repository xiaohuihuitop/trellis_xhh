# 命名约定

> 自研代码的命名是本规范里一致性要求最高的部分。命名漂移 = bug 源。

---

## 总则

- 所有自研标识符统一加 `xhh_` 前缀（`xhh` = 作者名缩写，是跨项目通用前缀，不是某个项目的专属名）
- 命名风格统一 snake_case（下划线小写）
- **缩写词一律全大写**：`LED` / `USB` / `BLE` / `ADC` / `UART` / `I2C` / `SPI` / `PWM`
  - 正确：`xhh_Task_LED_Init`、`xhh_Task_ADC_Cmd`
  - 错误：`xhh_Task_Led_Init`、`xhh_Task_adc_Cmd`
- 厂商代码（`SRC/`、`HAL/`、`LIB/`、`Profile/`）保持原命名，不统一改

---

## 函数命名

| 类别 | 形式 | 示例 |
|------|------|------|
| 模块接口 | `xhh_<层>_<模块>_<动词>` | `xhh_Task_Motor_Init` |
| 谓词（布尔查询） | `xhh_IS_<...>`，返回 `uint8_t` 0/1 | `xhh_IS_Can_KEY`、`xhh_IS_Motor_Run` |
| BSP 硬件操作 | `<模块>_BSP_Set_<对象>`（static，不进头文件） | `Motor_BSP_Set_EN`、`Motor_BSP_Set_Level` |
| 状态切换 | `xhh_SYS_Change` / `xhh_Mode_Change` | `xhh_SYS_Change(xhh_SYS_Run)` |
| 事件触发/处理 | `xhh_Event_Trigger` / `xhh_Event_Handle` | — |

动词后置约定（常用后缀）：

| 后缀 | 含义 |
|------|------|
| `_Init` / `_DeInit` | 模块初始化 / 反初始化 |
| `_Cmd(uint8_t)` | 使能控制（0=关，1=开） |
| `_Loop` | 主循环周期调用 |
| `_Set_*` / `_Get_*` | 写 / 读 对象字段 |
| `_Apply_*` | 把设置应用到硬件 |
| `_IS_*` | 谓词查询，返回 0/1 |

示例（真实代码）：

```c
// xhh_Module/xhh_Task/xhh_Task_Motor.h:86-92
void xhh_Task_Motor_Init(void);
void xhh_Task_Motor_DeInit(void);
void xhh_Task_Motor_Cmd(uint8_t cmd);
void xhh_Task_Motor_Apply_Mode_Fun(void);

// xhh_Module/xhh_Mode/xhh_Mode.h:94-102
uint8_t xhh_IS_Can_KEY(void);
uint8_t xhh_IS_OFF(void);
uint8_t xhh_IS_Motor_Run(void);
```

---

## 变量命名

| 类别 | 约定 | 示例 |
|------|------|------|
| 跨文件全局 | `g_` 前缀 | `g_xhh_user_data` |
| 模块内 static | 不加前缀，语义化命名 | `key_OK_step`、`holdpp_count` |
| 模块使能位 | `static volatile uint8_t xhh_task_<模块>_en` | `xhh_task_motor_en` |
| 中断/主循环共享 | 加 `volatile` | `volatile uint16_t tim_count` |
| 前一状态记录 | 后缀 `_f`（former） | `xhh_SYS_f` |

注意：使能位统一 `static volatile uint8_t`，不要漏 `static`（历史代码里 `xhh_task_holdpp_en` 漏了 static，新代码必须加）。

---

## 类型与宏

- 类型用 `typedef ... _t;`，后缀 `_t`
- 对象类型可去掉 `xhh_` 前缀，只保留 `_t`：`Motor_Obj_t`、`xhh_Event_t`
- 枚举值用模块前缀：`xhh_SYS_Run`、`MOTOR_MODE_NORMAL`、`LED_`...
- 宏全大写 + 下划线，常带模块前缀：`KEY_SHAKE_TIME`、`PROTOCOLS_SET_SYS`、`FLASH_USER_DATA_ADDR`

```c
// xhh_Module/xhh_Mode/xhh_Mode.h:19-32
typedef enum {
    xhh_SYS_Null = 0, xhh_SYS_Init, xhh_SYS_PowerON, xhh_SYS_PowerOFF,
    xhh_SYS_Run, xhh_SYS_Bright, xhh_SYS_Charge, xhh_SYS_ERR,
    xhh_SYS_Wake, xhh_SYS_PP_Start, xhh_SYS_PP_Stop
} xhh_SYS_t;
```

---

## 文件命名

- 小写 + 模块前缀，`.c/.h` 同名成对：`xhh_Task_Motor.c` / `xhh_Task_Motor.h`
- 层级体现：`xhh_<层>_<模块>.c`，如 `xhh_Event.c`、`xhh_Mode.c`、`xhh_Task_BAT.c`
- 聚合头：`xhh_Task_ALL.h`

---

## 头文件保护宏

- **只用 `#ifndef`，禁用 `#pragma once`**
- 统一格式：`__XHH_<MODULE>_H__`（双下划线包裹，全大写）

```c
// 正确
#ifndef __XHH_TASK_MOTOR_H__
#define __XHH_TASK_MOTOR_H__
...
#endif

// 错误（历史遗留三派混用，新代码不允许）
// INCLUDE_XHH_EVENT_H_
// XHH_TASK_XHH_TASK_BAT_H_
```

历史文件的保护宏若不属于 `__XHH_<MODULE>_H__` 形式，改动该文件时顺手统一。

---

## 初始化后应补充的项目事实

- 本项目实际模块前缀（若不用 `xhh_`）
- 实际缩写词清单
