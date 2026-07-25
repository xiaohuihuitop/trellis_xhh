// ===== 项目占位（复制后必须替换/确认已定义）=====
// xhh_Mode.h → 状态机头(声明 xhh_SYS_Change,见 xhh_Mode_Template)
// xhh_Task_UI_Set_ERR/xhh_Task_BAT_Cmd/xhh_Task_Key_Cmd/xhh_Task_Motor_Cmd → 各 Task 模块接口
// XHH_DEBUG → 日志宏(见 logging.md,在 xhh_BSP_Def.h 定义)

#include "xhh_Event_Template.h"
#include "xhh_Mode.h"
#include "xhh_BSP_SYS.h"

// ===== Event 私有单槽(事件值 + 参数) =====
static volatile xhh_Event_t xhh_event_slot = xhh_Event_Null;
static volatile uint32_t xhh_event_parameter_slot = xhh_Event_Parameter_ID_NULL;

// ===== 触发:写私有单槽 =====
void xhh_Event_Trigger(xhh_Event_t event, uint32_t xhh_Event_Parameter)
{
	xhh_BSP_SYS_IT_Disable();
	xhh_event_parameter_slot = xhh_Event_Parameter;
	xhh_event_slot = event;
	xhh_BSP_SYS_IT_Enable();
}

// ===== 分发:读后清零 + switch(主循环 10ms 调) =====
void xhh_Event_Handle(void)
{
	xhh_Event_t event_temp;
	uint32_t param;

	xhh_BSP_SYS_IT_Disable();
	event_temp = xhh_event_slot;
	param = xhh_event_parameter_slot;
	xhh_event_slot = xhh_Event_Null;          // AI:取出即清空，新事件留给下一轮 Handle。
	xhh_BSP_SYS_IT_Enable();

	if (event_temp == xhh_Event_Null)
		return;

	// AI:从局部快照拆参数，高16只用于来源判断，低16承载业务数据。
	uint32_t source_id = param & 0xFFFF0000UL;
	uint16_t data_16 = (uint16_t)(param & 0x0000FFFFUL);
	uint8_t data_h = (uint8_t)((data_16 >> 8) & 0xFFU);
	uint8_t data_l = (uint8_t)(data_16 & 0xFFU);
	(void)source_id;
	(void)data_h;
	(void)data_l;

	XHH_DEBUG("e_h:%d\r\n", event_temp);

	switch (event_temp)
	{
	case xhh_Event_Init:
		xhh_SYS_Change(xhh_SYS_Wake);
		break;

	case xhh_Event_PowerON:
		// 前置守卫(按需)
		// if (xhh_Mode_Get_N() != xhh_Mode_Work) break;
		// 多模块联动:集中设置,再切状态机
		// xhh_Task_Motor_Set_*(...);
		// xhh_Task_LED_Set_*(...);
		// xhh_Task_TIMEOUT_Set_*(...);
		xhh_SYS_Change(xhh_SYS_Run);
		break;

	case xhh_Event_TimeOut:
		if (source_id == xhh_Event_Parameter_ID_Touch)
		{
			// TODO: 使用 data_16、data_h 或 data_l 处理 Touch 来源的超时事件。
		}
		break;

	case xhh_Event_ERR:
		xhh_Task_UI_Set_ERR();
		xhh_Task_BAT_Cmd(0);
		xhh_Task_Key_Cmd(0);
		xhh_Task_Motor_Cmd(0);
		xhh_SYS_Change(xhh_SYS_ERR);
		break;

	// TODO: 加你的事件 case

	default:
		break;
	}
}
