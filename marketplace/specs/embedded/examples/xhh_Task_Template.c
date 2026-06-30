#include "xhh_Task_Template.h"
#include "xhh_Task_ALL.h"
#include "xhh_BSP_GPIO.h"   // 硬件走 BSP 公共层(见 bsp.md),不直接调厂商 API

// ===== 模块内状态(static 封装) =====
static volatile uint8_t xhh_task_template_en = 0;   // 使能位(必须 static volatile)
static Template_Obj_t Template = {0};               // 模块对象

// ===== 业务参数→硬件值换算放 Task 内 static(见 bsp.md) =====
static uint16_t Template_ValueToDuty(uint8_t value);

// ===== 四件套 =====

void xhh_Task_Template_Init(void)
{
	// GPIO 初始化已由 BSP_Init() 统一调 xhh_BSP_GPIO_Init() 完成,这里只做模块自身初始化
	// 如需 PWM 等其他 BSP,按需调 xhh_BSP_PWM_Init(xhh_BSP_PWM_ID_TEMPLATE_OUT);
}

void xhh_Task_Template_DeInit(void)
{
	// 反初始化:关硬件输出
	xhh_BSP_GPIO_Write(xhh_BSP_GPIO_ID_TEMPLATE_EN, 0);
}

void xhh_Task_Template_Cmd(uint8_t cmd)
{
	if (cmd)
	{
		xhh_task_template_en = 1;
	}
	else
	{
		xhh_task_template_en = 0;
		xhh_BSP_GPIO_Write(xhh_BSP_GPIO_ID_TEMPLATE_EN, 0);   // 关时复位硬件
	}
}

/// 周期调用(在 main_task.c 的 10ms/100ms 事件里调)
void xhh_Task_Template_Loop(void)
{
	if (xhh_task_template_en == 0)    // 首句守卫(必须)
		return;

	// TODO: 填模块周期逻辑
	// 硬件输出经 BSP,业务参数已在内部换算成 duty/level
	xhh_BSP_GPIO_Write(xhh_BSP_GPIO_ID_TEMPLATE_OUT, Template.value > 0 ? 1 : 0);
}

// ===== Set/Get =====

void xhh_Task_Template_Set_Obj(Template_Obj_t *obj)
{
	if (obj == NULL)                  // guard early
		return;
	Template = *obj;
}

Template_Obj_t xhh_Task_Template_Get_Obj(void)
{
	return Template;
}

// ===== 业务参数→硬件值换算(放 Task 内,不进 BSP) =====

static uint16_t Template_ValueToDuty(uint8_t value)
{
	// TODO: 按 PWM duty 范围换算 value(0~100)→duty
	(void)value;
	return 0;
}