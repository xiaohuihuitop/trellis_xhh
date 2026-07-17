// ===== 项目占位（复制后必须替换/确认已定义）=====
// xhh_Task_ALL.h → 聚合头(否则下方各 Task _Cmd 调用编译不过,见 xhh_Task_All_Template)

#ifndef XHH_EVENT_H
#define XHH_EVENT_H

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
/// @brief 触发事件(写全局单槽,新事件覆盖未处理的旧事件)
/// @param event 事件枚举值
/// @param xhh_Event_Parameter 参数(高16来源ID / 低16数据)
void xhh_Event_Trigger(xhh_Event_t event, uint32_t xhh_Event_Parameter);

/// @brief 分发事件(主循环 10ms 调:取出即清零 + 提取参数 + switch)
void xhh_Event_Handle(void);

#endif
