// ===== 项目占位（复制后必须替换/确认已定义）=====
// 复制后改名为 xhh_Task.c(见 directory-structure.md: xhh_Task.c 是聚合实现)
// 每新增 Task 模块时:在 xhh_Task_ALL.h 加 include,在下方 Init/Cmd/DeInit 里加转发

#include "xhh_Task_All_Template.h"

/// @brief 初始化所有 Task 模块(逐个转发)
void xhh_Task_ALL_Init(void)
{
	xhh_Task_ADC_Init();
	xhh_Task_Key_Init();
	xhh_Task_Motor_Init();
	xhh_Task_Flash_Init();
	// TODO: 加你的 Task 模块 _Init
}

/// @brief 反初始化所有 Task 模块(逐个转发)
void xhh_Task_ALL_DeInit(void)
{
	xhh_Task_ADC_DeInit();
	xhh_Task_Key_DeInit();
	xhh_Task_Motor_DeInit();
	xhh_Task_Flash_DeInit();
	// TODO: 加你的 Task 模块 _DeInit
}

/// @brief 使能控制所有 Task 模块
/// @param en 0=全关,1=全开
void xhh_Task_ALL_Cmd(uint8_t en)
{
	xhh_Task_ADC_Cmd(en);
	xhh_Task_Key_Cmd(en);
	xhh_Task_Motor_Cmd(en);
	xhh_Task_Flash_Cmd(en);
	// TODO: 加你的 Task 模块 _Cmd
}
