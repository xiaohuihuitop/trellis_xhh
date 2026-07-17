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
 * @note AI:由 APP 每 100ms 调用一次。
 */
void xhh_Task_HoldPP_Loop(void);
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
