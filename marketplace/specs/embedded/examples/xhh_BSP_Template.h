#ifndef XHH_BSP_TEMPLATE_H
#define XHH_BSP_TEMPLATE_H

#include "xhh_BSP_Def.h"

/**
 * @brief AI:当前项目使用的稳定逻辑信号名。
 * @note AI:枚举值不代表厂家端口、引脚或有效电平。
 */
typedef enum
{
	xhh_BSP_TEMPLATE_INPUT_ACTIVE = 0,
	xhh_BSP_TEMPLATE_OUTPUT_ENABLE
} xhh_BSP_TEMPLATE_Signal_t;

/**
 * @brief AI:初始化当前项目使用的全部 Template 硬件资源。
 * @param 无。
 * @return 无。
 */
void xhh_BSP_TEMPLATE_Init(void);

/**
 * @brief AI:读取逻辑输入信号是否有效。
 * @param signal 逻辑输入信号名。
 * @retval 0 信号无效。
 * @retval 1 信号有效。
 */
uint8_t xhh_BSP_TEMPLATE_Read(xhh_BSP_TEMPLATE_Signal_t signal);

/**
 * @brief AI:设置逻辑输出信号的有效状态。
 * @param signal 逻辑输出信号名。
 * @param active 0 表示无效，非 0 表示有效。
 * @return 无。
 */
void xhh_BSP_TEMPLATE_Write(xhh_BSP_TEMPLATE_Signal_t signal, uint8_t active);

#endif
