#include "xhh_Mode.h"
#include "xhh_Event_Template.h"
#include "xhh_Task_ALL.h"

// ===== 状态枚举 =====
typedef enum
{
	xhh_SYS_Null = 0,
	xhh_SYS_Init,
	xhh_SYS_PowerON,
	xhh_SYS_PowerOFF,
	xhh_SYS_Run,
	xhh_SYS_Charge,
	xhh_SYS_ERR,
	xhh_SYS_Wake,
	// TODO: 加你的状态
} xhh_SYS_t;

// ===== 子步枚举 =====
typedef enum
{
	xhh_SYS_Step_Entry = 0,
	xhh_SYS_Step_Ing,
	xhh_SYS_Step_Done,
} xhh_SYS_Step_t;

// ===== 状态机变量 =====
xhh_SYS_t xhh_SYS_n = xhh_SYS_Null;
xhh_SYS_t xhh_SYS_f = xhh_SYS_Null;          // 前一状态(former)
xhh_SYS_Step_t xhh_SYS_Step_n = xhh_SYS_Step_Entry;
uint16_t xhh_SYS_Loop_Count = 0;

// ===== 集中转换(状态切换只走这里) =====
void xhh_SYS_Change(xhh_SYS_t new_sys)
{
	xhh_SYS_f = xhh_SYS_n;                    // 保存前一状态
	xhh_SYS_n = new_sys;
	xhh_SYS_Step_n = xhh_SYS_Step_Entry;      // 重置子步
	xhh_SYS_Loop_Count = 0;                   // 重置计数
	XHH_DEBUG("s_h:%d-->%d\r\n", xhh_SYS_f, xhh_SYS_n);
}

// ===== 状态机推进(主循环 10ms 调) =====
void xhh_SYS_Handle(void)
{
	switch (xhh_SYS_n)
	{
	case xhh_SYS_Wake:
		if (xhh_SYS_Step_n == xhh_SYS_Step_Entry)
		{
			xhh_Task_Key_Cmd(1);
			xhh_Task_ADC_Cmd(1);
			xhh_SYS_Loop_Count = 0;
			xhh_SYS_Step_n++;                 // Entry 做完进 Ing
		}
		else if (xhh_SYS_Step_n == xhh_SYS_Step_Ing)
		{
			if (xhh_SYS_Loop_Count < 1000)
				xhh_SYS_Loop_Count++;
			if (xhh_SYS_Loop_Count >= 200)
				xhh_SYS_Change(xhh_SYS_PowerON);
		}
		break;

	case xhh_SYS_PowerON:
		if (xhh_SYS_Step_n == xhh_SYS_Step_Entry)
		{
			// TODO: 上电初始化(BLE 广播、开模块等)
			xhh_SYS_Step_n++;
		}
		else if (xhh_SYS_Step_n == xhh_SYS_Step_Ing)
		{
			// TODO: 等待条件后切 Run
			xhh_SYS_Change(xhh_SYS_Run);
		}
		break;

	case xhh_SYS_Run:
		if (xhh_SYS_Step_n == xhh_SYS_Step_Entry)
		{
			// TODO: 进入运行态的初始化
			xhh_SYS_Step_n++;
		}
		else if (xhh_SYS_Step_n == xhh_SYS_Step_Ing)
		{
			// TODO: 运行态持续逻辑
		}
		break;

	case xhh_SYS_PowerOFF:
		if (xhh_SYS_Step_n == xhh_SYS_Step_Entry)
		{
			// 关机:收集运行时 → 存 Flash
			xhh_Task_Flash_Update_Flash_Data(&g_xhh_user_data);
			xhh_Task_Flash_Save_User_Data(&g_xhh_user_data);
			xhh_Task_ALL_Cmd(0);
			xhh_SYS_Step_n++;
		}
		else if (xhh_SYS_Step_n == xhh_SYS_Step_Ing)
		{
			// TODO: 等待关机完成
		}
		break;

	case xhh_SYS_ERR:
		if (xhh_SYS_Step_n == xhh_SYS_Step_Entry)
		{
			// TODO: 错误态处理
			xhh_SYS_Step_n++;
		}
		break;

	// TODO: 加你的状态 case

	default:
		break;
	}
}

// ===== 谓词(按需) =====
uint8_t xhh_IS_OFF(void)
{
	return (xhh_SYS_n == xhh_SYS_PowerOFF) ? 1 : 0;
}

uint8_t xhh_IS_Run(void)
{
	return (xhh_SYS_n == xhh_SYS_Run) ? 1 : 0;
}
