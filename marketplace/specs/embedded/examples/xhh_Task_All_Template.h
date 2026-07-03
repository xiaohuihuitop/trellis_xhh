// ===== 项目占位（复制后必须替换/确认已定义）=====
// 复制后改名为 xhh_Task_ALL.h(见 directory-structure.md)
// 每新增 Task 模块时:在下方 include 区加它的头,在 xhh_Task_All_Template.c 的 Init/Cmd/DeInit 里加转发

#ifndef __XHH_TASK_ALL_H__
#define __XHH_TASK_ALL_H__

// ===== 聚合 include 所有 Task 头(每新增一个 Task 都要加这里) =====
#include "xhh_Task_ADC.h"
#include "xhh_Task_Key.h"
#include "xhh_Task_Motor.h"
#include "xhh_Task_Flash.h"
// TODO: 加你的 Task 模块头
// #include "xhh_Task_<X>.h"

// ===== 聚合接口(主循环/状态机通过这层统一开关,不直接逐个调) =====
/// @brief 初始化所有 Task 模块(逐个转发 _Init)
void xhh_Task_ALL_Init(void);

/// @brief 反初始化所有 Task 模块(逐个转发 _DeInit)
void xhh_Task_ALL_DeInit(void);

/// @brief 使能控制所有 Task 模块
/// @param en 0=全关,1=全开
void xhh_Task_ALL_Cmd(uint8_t en);

#endif
