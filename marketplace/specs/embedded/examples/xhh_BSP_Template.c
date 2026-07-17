#include "xhh_BSP_Template.h"

#include "CONFIG.h"
#include "xhh_BSP_SYS.h"

#define XHH_BSP_TEMPLATE_INPUT_PIN GPIO_Pin_0
#define XHH_BSP_TEMPLATE_OUTPUT_PIN GPIO_Pin_1

/**
 * @brief AI:初始化当前平台固定的输入和输出引脚。
 * @param 无。
 * @return 无。
 * @note AI:更换 MCU 或板卡时只修改本文件的厂家资源和初始化参数。
 */
void xhh_BSP_TEMPLATE_Init(void)
{
	GPIOA_ModeCfg(XHH_BSP_TEMPLATE_INPUT_PIN, GPIO_ModeIN_PU);
	GPIOA_ResetBits(XHH_BSP_TEMPLATE_OUTPUT_PIN);
	GPIOA_ModeCfg(XHH_BSP_TEMPLATE_OUTPUT_PIN, GPIO_ModeOut_PP_5mA);
}

/**
 * @brief AI:读取当前平台固定的低电平有效输入。
 * @param signal 逻辑输入信号名。
 * @retval 0 信号无效。
 * @retval 1 信号有效。
 */
uint8_t xhh_BSP_TEMPLATE_Read(xhh_BSP_TEMPLATE_Signal_t signal)
{
	if (signal != xhh_BSP_TEMPLATE_INPUT_ACTIVE)
	{
		xhh_BSP_SYS_ERR_Handle();
	}

	return GPIOA_ReadPortPin(XHH_BSP_TEMPLATE_INPUT_PIN) == 0U;
}

/**
 * @brief AI:设置当前平台固定的高电平有效输出。
 * @param signal 逻辑输出信号名。
 * @param active 0 表示无效，非 0 表示有效。
 * @return 无。
 */
void xhh_BSP_TEMPLATE_Write(xhh_BSP_TEMPLATE_Signal_t signal, uint8_t active)
{
	if (signal != xhh_BSP_TEMPLATE_OUTPUT_ENABLE)
	{
		xhh_BSP_SYS_ERR_Handle();
		return;
	}

	if (active != 0U)
	{
		GPIOA_SetBits(XHH_BSP_TEMPLATE_OUTPUT_PIN);
	}
	else
	{
		GPIOA_ResetBits(XHH_BSP_TEMPLATE_OUTPUT_PIN);
	}
}
