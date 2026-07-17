#include "xhh_Task_Template.h"

#include "xhh_BSP_GPIO.h"

static volatile uint8_t xhh_task_template_en = 0U;
static Template_Obj_t template_obj = {TEMPLATE_MODE_A, TEMPLATE_VALUE_MIN};

void xhh_Task_Template_Cmd(uint8_t cmd)
{
	xhh_task_template_en = cmd;
	if (cmd == 0U)
	{
		xhh_BSP_GPIO_Write(xhh_BSP_GPIO_TEMPLATE_ENABLE, 0U);
	}
}

void xhh_Task_Template_Loop(void)
{
	if (xhh_task_template_en == 0U)
	{
		return;
	}

	xhh_BSP_GPIO_Write(xhh_BSP_GPIO_TEMPLATE_ENABLE,
						   template_obj.value > TEMPLATE_VALUE_MIN);
}

void xhh_Task_Template_Set_Obj(const Template_Obj_t *obj)
{
	if (obj == NULL)
	{
		return;
	}
	template_obj = *obj;
}

Template_Obj_t xhh_Task_Template_Get_Obj(void)
{
	return template_obj;
}
