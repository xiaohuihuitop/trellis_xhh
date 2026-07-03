// ===== 项目占位（复制后必须替换/确认已定义）=====
// 复制后改名为 xhh_Mode.h(配对 xhh_Mode.c)
// 把 xhh_SYS_t 枚举值换成本项目真实状态全集(见 state-machine.md "初始化后应补充的项目事实")

#ifndef __XHH_MODE_H__
#define __XHH_MODE_H__

#include <stdint.h>

// ===== 状态枚举(定义在 .h,因为 xhh_SYS_Change 参数要用) =====
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
	xhh_SYS_Step_Entry = 0,   // 进入状态首做的事
	xhh_SYS_Step_Ing,         // 持续运行/等待
	xhh_SYS_Step_Done,        // 收尾(可选)
} xhh_SYS_Step_t;

// ===== 状态机变量(定义在 .c,这里 extern) =====
extern xhh_SYS_t xhh_SYS_n;
extern xhh_SYS_t xhh_SYS_f;              // 前一状态(former)
extern xhh_SYS_Step_t xhh_SYS_Step_n;
extern uint16_t xhh_SYS_Loop_Count;

// ===== 接口 =====
/// @brief 状态切换(唯一入口,同时保存前一状态 + 重置子步和计数)
/// @param new_sys 新状态
void xhh_SYS_Change(xhh_SYS_t new_sys);

/// @brief 状态机推进(主循环 10ms 调,switch 状态 + 分支子步)
void xhh_SYS_Handle(void);

// ===== 谓词(按需,返回 uint8_t 0/1) =====
/// @brief 当前是否关机态
uint8_t xhh_IS_OFF(void);

/// @brief 当前是否运行态
uint8_t xhh_IS_Run(void);

#endif
