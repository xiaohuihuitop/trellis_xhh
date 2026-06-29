#include "xhh_Task_Template.h"
#include "xhh_Task_ALL.h"

// ===== 模块内状态(static 封装) =====
static volatile uint8_t xhh_task_template_en = 0;   // 使能位(必须 static volatile)
static Template_Obj_t Template = {0};               // 模块对象

// ===== BSP static 前向声明(不进头文件) =====
static void Template_BSP_Set_EN(uint8_t set);
static void Template_BSP_Set_Value(uint8_t value);

// ===== 四件套 =====

void xhh_Task_Template_Init(void)
{
	// GPIO / 外设配置(按实际引脚改)
	GPIOB_ResetBits(GPIO_Pin_7);
	GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);
}

void xhh_Task_Template_DeInit(void)
{
	// 反初始化:关输出 + 复位 GPIO
	Template_BSP_Set_EN(0);
	GPIOB_ResetBits(GPIO_Pin_7);
	GPIOB_ModeCfg(GPIO_Pin_7, GPIO_ModeOut_PP_5mA);
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
		Template_BSP_Set_EN(0);    // 关时复位硬件
	}
}

/// 周期调用(在 main_task.c 的 10ms/100ms 事件里调)
void xhh_Task_Template_Loop(void)
{
	if (xhh_task_template_en == 0)    // 首句守卫(必须)
		return;

	// TODO: 填模块周期逻辑
	Template_BSP_Set_Value(Template.value);
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

// ===== BSP static 实现(硬件细节,不进头文件) =====

static void Template_BSP_Set_EN(uint8_t set)
{
	if (set)
		GPIOB_SetBits(GPIO_Pin_7);
	else
		GPIOB_ResetBits(GPIO_Pin_7);
}

static void Template_BSP_Set_Value(uint8_t value)
{
	// TODO: 填硬件输出(PWM/DAC/寄存器等)
	(void)value;
}
