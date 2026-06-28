#include "xhh_Task_Flash.h"
#include "xhh_Task_ALL.h"

// ===== 持久化结构体(所有用户数据集中) =====
#define FLASH_USER_DATA_ADDR 0

typedef struct
{
	uint16_t time;                  // 运行时长
	xhh_Mode_t mode;                // 当前模式
	Motor_Obj_t motor;              // 电机对象
	uint8_t temp_max;               // TODO: 加你的字段
} xhh_Task_Flash_user_data_t;

xhh_Task_Flash_user_data_t g_xhh_user_data;   // 跨文件全局(唯一运行副本)

// ===== 有效性校验(逐字段范围检查) =====
uint8_t xhh_Task_Flash_User_Data_IS_Valid(xhh_Task_Flash_user_data_t *data)
{
	if (data == NULL)
		return 0;
	if (data->mode < MODE_MIN || data->mode > MODE_MAX)
		return 0;
	if (data->motor.level < MOTOR_LEVEL_MIN || data->motor.level > MOTOR_LEVEL_MAX)
		return 0;
	if (data->temp_max < TEMPLATE_VALUE_MIN || data->temp_max > TEMPLATE_VALUE_MAX)
		return 0;
	// TODO: 加你的字段校验
	return 1;
}

// ===== 清默认值(非法数据整体恢复) =====
void xhh_Task_Flash_User_Data_Clean(xhh_Task_Flash_user_data_t *data)
{
	if (data == NULL)
		return;
	data->time = 0;
	data->mode = MODE_DEFAULT;
	data->motor.mode = MOTOR_MODE_NORMAL;
	data->motor.level[0] = LEVEL_SAVE_1;
	data->motor.level[1] = LEVEL_SAVE_2;
	data->temp_max = TEMPLATE_VALUE_MAX;       // TODO: 填你的默认值
}

// ===== Flash → 结构体 =====
void xhh_Task_Flash_Get_User_Data(xhh_Task_Flash_user_data_t *data)
{
	if (data == NULL)
		return;
	Flash_Read((uint8_t *)data, sizeof(*data), FLASH_USER_DATA_ADDR);
	if (xhh_Task_Flash_User_Data_IS_Valid(data) == 0)
	{
		xhh_Task_Flash_User_Data_Clean(data);
		xhh_Task_Flash_Save_User_Data(data);
	}
}

// ===== 结构体 → Flash =====
void xhh_Task_Flash_Save_User_Data(xhh_Task_Flash_user_data_t *data)
{
	if (data == NULL)
		return;
	if (xhh_Task_Flash_User_Data_IS_Valid(data) == 0)
		xhh_Task_Flash_User_Data_Clean(data);
	Flash_Write((uint8_t *)data, sizeof(*data), FLASH_USER_DATA_ADDR);
}

// ===== 结构体 → 各 Task 运行时对象(开机时) =====
void xhh_Task_Flash_Update_User_Data(xhh_Task_Flash_user_data_t *data)
{
	if (data == NULL)
		return;
	// TODO: 把 data 各字段推到对应 Task 的运行时对象
	// xhh_Task_Motor_Set_Obj(&data->motor);
	// xhh_Mode_Set_N(data->mode);
}

// ===== 各 Task 运行时对象 → 结构体(关机时) =====
void xhh_Task_Flash_Update_Flash_Data(xhh_Task_Flash_user_data_t *data)
{
	if (data == NULL)
		return;
	// TODO: 从各 Task 收集到结构体
	// data->motor = xhh_Task_Motor_Get_Obj();
	// data->mode = xhh_Mode_Get_N();
	data->time = 0;   // 瞬态不存,按需改
}

// ===== 四件套(Flash 模块自己的) =====
void xhh_Task_Flash_Init(void)
{
	xhh_Task_Flash_Get_User_Data(&g_xhh_user_data);
	xhh_Task_Flash_Update_User_Data(&g_xhh_user_data);
}

void xhh_Task_Flash_DeInit(void)
{
}

void xhh_Task_Flash_Cmd(uint8_t cmd)
{
	(void)cmd;   // Flash 模块通常常开,cmd 按需用
}

void xhh_Task_Flash_Loop(void)
{
	// Flash 模块通常无周期任务;关机时由状态机调 Save
}
