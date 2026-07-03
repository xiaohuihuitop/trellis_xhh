// ===== 项目占位（复制后必须替换/确认已定义）=====
// xhh_Task.h → Task 公共头(声明 xhh_Task_ALL 等,项目要有)
// TEMPLATE_VALUE_MIN/MAX → 本 .h 内已定义,按需改范围

#ifndef __XHH_TASK_TEMPLATE_H__
#define __XHH_TASK_TEMPLATE_H__

#include "xhh_Task.h"

// ===== 宏(按需) =====
#define TEMPLATE_VALUE_MIN  0
#define TEMPLATE_VALUE_MAX  100

// ===== 枚举(按需) =====
typedef enum
{
	TEMPLATE_MODE_Null = 0,
	TEMPLATE_MODE_A = 1,
	TEMPLATE_MODE_B,
	TEMPLATE_MODE_ADD = 0xff,
	TEMPLATE_MODE_SUB = 0xee,
} Template_Mode_t;

// ===== 对象结构体(按需,若模块需记忆状态) =====
typedef struct
{
	Template_Mode_t mode;
	uint8_t value;
} Template_Obj_t;

// ===== 四件套(必须) =====
/// @brief 模块初始化(GPIO 初始化由 BSP_Init() 统一完成,这里只做模块自身初始化)
void xhh_Task_Template_Init(void);

/// @brief 模块反初始化(释放资源,关硬件输出)
void xhh_Task_Template_DeInit(void);

/// @brief 使能控制
/// @param cmd 0=关,1=开
void xhh_Task_Template_Cmd(uint8_t cmd);

/// @brief 主循环周期调用(跑模块逻辑,首句有使能位守卫)
void xhh_Task_Template_Loop(void);

// ===== Set/Get(按需) =====
/// @brief 设置模块对象
/// @param obj 对象指针,NULL 则忽略(guard early)
void xhh_Task_Template_Set_Obj(Template_Obj_t *obj);

/// @brief 获取模块对象
/// @return 模块对象副本
Template_Obj_t xhh_Task_Template_Get_Obj(void);

#endif
