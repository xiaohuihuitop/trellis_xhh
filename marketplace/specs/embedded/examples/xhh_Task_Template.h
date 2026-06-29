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
void xhh_Task_Template_Init(void);
void xhh_Task_Template_DeInit(void);
void xhh_Task_Template_Cmd(uint8_t cmd);
void xhh_Task_Template_Loop(void);

// ===== Set/Get(按需) =====
void xhh_Task_Template_Set_Obj(Template_Obj_t *obj);
Template_Obj_t xhh_Task_Template_Get_Obj(void);

#endif
