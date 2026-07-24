#include "xhh_Task_Template.h"

static volatile uint8_t xhh_task_template_en = 0U;
static Template_Obj_t template_obj = {TEMPLATE_MODE_A, TEMPLATE_VALUE_MIN};

/* AI:硬件操作接口。Task_Template 中所有 BSP 输出只能集中在本区域。 */
static void xhh_Task_Template_Output(void)
{
	if (xhh_task_template_en == 0U)
	{
		/* AI:复制后通过已确认的 BSP 接口关闭输出。 */
		return;
	}

	/* AI:复制后通过已确认的 BSP 接口应用 template_obj。 */
}

void xhh_Task_Template_Cmd(uint8_t cmd)
{
	xhh_task_template_en = cmd;
	xhh_Task_Template_Output();
}

void xhh_Task_Template_Set_Obj(const Template_Obj_t *obj)
{
	if (obj == NULL)
	{
		return;
	}
	template_obj = *obj;
}

void xhh_Task_Template_Apply(void)
{
	xhh_Task_Template_Output();
}

Template_Obj_t xhh_Task_Template_Get_Obj(void)
{
	return template_obj;
}

void xhh_Task_Template_Loop(void)
{
	if (xhh_task_template_en == 0U)
	{
		return;
	}

	/* AI:仅在周期相位或目标值变化时调用 xhh_Task_Template_Output，不能每个周期重复输出。 */
}
