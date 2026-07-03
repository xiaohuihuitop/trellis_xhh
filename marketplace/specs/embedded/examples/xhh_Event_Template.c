// ===== 项目占位（复制后必须替换/确认已定义）=====
// xhh_Mode.h → 状态机头(声明 xhh_SYS_Change,见 xhh_Mode_Template)
// xhh_Task_UI_Set_ERR/xhh_Task_BAT_Cmd/xhh_Task_Key_Cmd/xhh_Task_Motor_Cmd → 各 Task 模块接口
// XHH_DEBUG → 日志宏(见 logging.md,在 main.h 定义)

#include "xhh_Event_Template.h"
#include "xhh_Mode.h"

// ===== 全局单槽(事件值 + 参数) =====
xhh_Event_t Event_n = xhh_Event_Null;
uint32_t xhh_Event_Parameter_n = xhh_Event_Parameter_ID_NULL;

// ===== 触发:写全局量 =====
void xhh_Event_Trigger(xhh_Event_t event, uint32_t xhh_Event_Parameter)
{
	Event_n = event;
	xhh_Event_Parameter_n = xhh_Event_Parameter;
	XHH_DEBUG("e_t:%d\r\n", event);
}

// ===== 分发:读后清零 + switch(主循环 10ms 调) =====
void xhh_Event_Handle(void)
{
	xhh_Event_t event_temp = Event_n;
	Event_n = xhh_Event_Null;                 // 取出即清零
	uint32_t param = xhh_Event_Parameter_n;   // 先存参数(见 event-system.md),再清零
	xhh_Event_Parameter_n = xhh_Event_Parameter_ID_NULL;

	if (event_temp == xhh_Event_Null)
		return;

	// 拆参数(高16来源 / 低16数据)——从 param 提取,不是从已清零的全局
	uint16_t data_16 = (uint16_t)(param & 0xffff);
	uint8_t data_h = (uint8_t)((data_16 >> 8) & 0xff);
	uint8_t data_l = (uint8_t)(data_16 & 0xff);
	(void)data_h; (void)data_l;               // 按需用

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
