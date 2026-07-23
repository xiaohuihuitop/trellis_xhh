#ifndef XHH_TASK_TEMPLATE_H
#define XHH_TASK_TEMPLATE_H

#include "xhh_Task.h"

#define TEMPLATE_VALUE_MIN 0U
#define TEMPLATE_VALUE_MAX 100U

typedef enum
{
	TEMPLATE_MODE_Null = 0,
	TEMPLATE_MODE_A,
	TEMPLATE_MODE_B,
	TEMPLATE_MODE_ADD = 0xff,
	TEMPLATE_MODE_SUB = 0xee
} Template_Mode_t;

typedef struct
{
	Template_Mode_t mode;
	uint8_t value;
} Template_Obj_t;

/**
 * @brief AI:使能或关闭 Template Task。
 * @param cmd 0 关闭，非 0 使能。
 * @return 无。
 */
void xhh_Task_Template_Cmd(uint8_t cmd);

/**
 * @brief AI:设置模块对象。
 * @param obj 对象指针，不能为空。
 * @return 无。
 */
void xhh_Task_Template_Set_Obj(const Template_Obj_t *obj);

/**
 * @brief AI:按当前私有参数更新 Template 硬件输出。
 * @param 无。
 * @return 无。
 * @note AI:调用方在 Set_Obj 后需要即时生效时，显式调用本接口。
 */
void xhh_Task_Template_Apply(void);

/**
 * @brief AI:获取模块对象副本。
 * @param 无。
 * @return 当前模块对象。
 */
Template_Obj_t xhh_Task_Template_Get_Obj(void);

/**
 * @brief AI:执行 Template Task 周期逻辑。
 * @param 无。
 * @return 无。
 * @note AI:由 APP 每 10ms 调用一次；实现必须是 Task 源文件中的最后一个函数定义。
 */
void xhh_Task_Template_Loop(void);

#endif
