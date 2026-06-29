#ifndef __XHH_EVENT_H__
#define __XHH_EVENT_H__

#include "xhh_Task_ALL.h"

// ===== 事件枚举 =====
typedef enum
{
	xhh_Event_Null = 0,
	xhh_Event_Init,
	xhh_Event_PowerON,
	xhh_Event_PowerOFF,
	xhh_Event_Change_Mode,
	xhh_Event_Level_ADD,
	xhh_Event_Level_SUB,
	xhh_Event_TimeOut,
	xhh_Event_BATLow,
	xhh_Event_ERR,
	// TODO: 加你的事件
} xhh_Event_t;

// ===== 事件参数 ID(高 16bit 来源 / 低 16bit 数据) =====
#define xhh_Event_Parameter_ID_NULL  0x00000000
#define xhh_Event_Parameter_ID_BLE   0x11110000
#define xhh_Event_Parameter_ID_Touch 0x22220000
#define xhh_Event_Parameter_ID_Key   0x33330000

// ===== 接口 =====
void xhh_Event_Trigger(xhh_Event_t event, uint32_t xhh_Event_Parameter);
void xhh_Event_Handle(void);

#endif
