# Task 模块模式

> 每个功能域对应一个 `xhh_Task_<X>.c/.h`。Task 只维护业务状态、处理命令和执行周期逻辑；全部硬件初始化由对应 `xhh_BSP_*_Init` 完成。

---

## Task 接口边界

Task 不提供 `_Init` / `_DeInit`。业务控制和周期处理按以下规则组织：

- 每个纳入 Task 聚合层的模块提供 `xhh_Task_<X>_Cmd(uint8_t cmd)`，用于统一业务使能控制。
- 需要周期处理时提供 `xhh_Task_<X>_Loop(void)`，并在 Doxygen 中声明调用周期。
- 按真实业务需要提供 `_Set_*`、`_Get_*`、`_Apply_*`、`_IS_*`。
- 为统一 Cmd 调度允许保留明确占位的 `_Cmd`；函数体必须使用 `AI:` 注释说明原因、当前无动作行为和启用条件。

```c
void xhh_Task_HoldPP_Cmd(uint8_t cmd);

/**
 * @brief AI:执行保持供电周期处理。
 * @note AI:由 APP 每 10ms 调用一次。
 */
void xhh_Task_HoldPP_Loop(void);
```

---

## 参数设置与输出应用（强制）

当一个 Task 的输出由“业务参数 + 当前模式/状态”共同决定时，参数设置和输出应用必须分离：

- `_Set_*` 只校验并更新 Task 私有参数、表下标或配置状态；不得调用 `_Set_Mode_Fun`、`_Apply_*`、目标更新函数、硬件输出接口或 `xhh_BSP_*`。
- `_Set_Mode_Fun` 或明确的 `_Apply_*` 是参数组合到业务状态、目标值和即时输出的唯一公开应用入口；调用方先 `_Set_*`，再调用该入口。
- Event 需要改变参数并即时生效时，case 内显式写出“`_Set_*` → `_Set_Mode_Fun`/`_Apply_*`”两个调用，不把第二步隐藏在 `_Set_*` 中。
- `_Loop` 只在彩虹、呼吸、闪烁等内部相位实际变化时更新目标并触发输出，不能每个周期重复写相同硬件值。
- `_Cmd` 为生命周期例外：关闭或复位时可立即应用安全输出。

```c
void xhh_Task_RGB_Set_Select(RGB_Select_t select)
{
	/* AI:只更新 rgb_select 和彩虹下标，不更新目标颜色。 */
}

void xhh_Task_RGB_Set_Mode_Fun(xhh_Mode_t mode, xhh_Mode_Fun_t mode_fun)
{
	/* AI:组合 Select 与 Mode，更新状态和目标颜色。 */
}

case xhh_Event_RGB_Select_ADD:
	xhh_Task_RGB_Set_Select(RGB_Select_ADD);
	xhh_Task_RGB_Set_Mode_Fun(xhh_Mode_Get_N(), xhh_Mode_Fun_Get_N());
	break;
```

---

## 函数定义顺序（强制）

`xhh_Task_<X>.c` 的函数定义按以下顺序组织，便于先定位硬件输出，再阅读业务状态和周期逻辑：

1. **硬件操作接口**：所有直接调用 `xhh_BSP_*` 写、启动、停止或设置输出的 `static` 函数放在最前面；前方必须用 `AI:硬件操作接口` 标注。
2. **Cmd 接口**：`xhh_Task_<X>_Cmd(uint8_t cmd)` 紧随硬件操作接口，集中完成使能位、状态和计数器复位。
3. **其他公开接口**：`_Set_*`、`_Get_*`、`_Apply_*`、`_IS_*` 按头文件声明顺序定义。
4. **Loop 接口**：`xhh_Task_<X>_Loop(void)` 必须是文件中最后一个函数定义。

没有硬件输出的 Task 直接从 `Cmd` 开始。简单的状态复位、状态赋值和周期分支收拢在所属公开接口中，不增加私有辅助函数打断以上顺序。确有无法内联的复杂纯算法时，需单独说明其职责，并放在硬件操作接口之后、`Cmd` 之前；不得调用硬件。

```c
/* AI:硬件操作接口。Task 内所有 BSP 输出只能集中在本区域。 */
static void xhh_Task_LED_Output_One(LED_Index_t index)
{
	xhh_BSP_GPIO_Write(xhh_BSP_GPIO_LED_1_ENABLE, index == LED_Index_1);
}

void xhh_Task_LED_Cmd(uint8_t cmd)
{
	/* AI:复位 Task 私有状态。 */
}

void xhh_Task_Template_Set_Value(uint8_t value)
{
	/* AI:只更新 Task 私有参数，不输出硬件。 */
}

void xhh_Task_Template_Apply(void)
{
	/* AI:组合参数与当前状态后调用硬件输出接口。 */
}

void xhh_Task_LED_Loop(void)
{
	/* AI:文件最后一个函数定义。 */
}
```

---

## 状态初始化与复位

- Task 初始状态直接使用 `.c` 内 `static` 变量初始化器表达。
- 模块重新使能、关闭时需要复位的计数器和业务状态，在 `_Cmd` 中明确处理。
- Task 不配置、启动或反初始化 GPIO、PWM、ADC、SPI、Timer 等硬件。
- 硬件初始化统一由 APP 调用对应 `xhh_BSP_*_Init` 完成。

```c
static volatile uint8_t xhh_task_motor_en = 0;
static uint16_t motor_delay_count = 0;

void xhh_Task_Motor_Cmd(uint8_t cmd)
{
	xhh_task_motor_en = cmd;
	motor_delay_count = 0;
	if (cmd == 0U)
	{
		xhh_BSP_GPIO_Write(xhh_BSP_GPIO_MOTOR_ENABLE, 0U);
	}
}
```

---

## 内部状态封装

- 模块状态放在 `.c` 内私有 `static` context 或 `static` 变量中。
- 禁止公开可写全局变量；确需跨模块共享时提供明确的 Get/Set 接口。
- 中断和主循环共享的状态才使用 `volatile`，不能把 `volatile` 当作并发保护。

---

## 周期接口守卫

受 `_Cmd` 控制的周期接口，入口必须检查使能状态；未使能时立即返回且不操作硬件：

```c
void xhh_Task_Motor_Loop(void)
{
	if (xhh_task_motor_en == 0U)
	{
		return;
	}

	/* AI:执行本周期业务逻辑。 */
}
```

周期由 APP 调度入口明确安排。接口名沿用 `_Loop` 时，头文件 Doxygen 必须写明 1ms、10ms、100ms 等实际调用周期。

---

## 硬件访问

Task 只能调用 `xhh_BSP_*` 公开接口，不出现厂商 API、寄存器、端口、真实引脚号或硬件初始化参数：

```c
xhh_BSP_GPIO_Write(xhh_BSP_GPIO_MOTOR_ENABLE, enable);
xhh_BSP_PWM1_Set_Duty(LevelToDutyCount(level));
```

- GPIO 使用 BSP 公开的稳定逻辑信号名；端口、引脚和有效极性只在 `xhh_BSP_GPIO.c`。
- ADC 使用 `xhh_BSP_ADC_CHANNEL_<n>` 逻辑通道名；厂家通道值只在 `xhh_BSP_ADC.c`。
- 业务参数到硬件计数值的换算仍放在 Task 私有函数中。
- Task 不提供 `Config` 接口接收硬件资源，也不保存板级端口和引脚配置。

---

## 聚合层 xhh_Task_ALL

`xhh_Task_ALL.h` 只聚合 Task 头文件和当前真实需要的统一业务操作。现阶段统一使能接口为：

```c
void xhh_Task_ALL_Cmd(uint8_t en);
```

`xhh_Task.c` 只转发各模块 `_Cmd`。不提供 `xhh_Task_ALL_Init` 或 `xhh_Task_ALL_DeInit`；硬件初始化由 APP 直接调用各 BSP Init。

新增 Task 时：

1. 新建 `xhh_Task_<X>.c/.h`。
2. 声明统一 Cmd，并按真实需要增加 Loop、Get/Set 等接口。
3. 在 `xhh_Task_ALL.h` 增加头文件聚合。
4. 模块需要统一使能时，才在 `xhh_Task_ALL_Cmd` 增加转发。

---

## 禁止

- Task 声明或实现 `_Init`、`_DeInit`。
- Task 提供 GPIO/PWM/ADC 等硬件 `Config` 接口。
- Task 保存真实端口、引脚、厂家通道或硬件初始化参数。
- 在周期接口中遗漏使能守卫。
- Task 直接调用厂家 API、寄存器或 include 厂家 SDK 头。
- 为接口形式统一增加 Task `_Init/_DeInit` 空函数。
- 占位 Cmd 没有在函数体内用 `AI:` 注释说明原因、当前行为和启用条件。
- 新增设备型 BSP 或 `xhh_Port_*` 转发层。
