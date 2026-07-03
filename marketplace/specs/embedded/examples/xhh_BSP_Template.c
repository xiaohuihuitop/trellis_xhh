// ===== 项目占位（复制后必须替换/确认已定义）=====
// 1. 顶部 include 的 CONFIG.h → 本项目平台头(WCH 用 CONFIG.h,PY32 用 py32f0xx_hal.h)
// 2. APP_TEMPLATE_*_PIN 物理宏 → 真实引脚号(查芯片数据手册)
// 3. GPIOA_ModeCfg/GPIOB_ModeCfg/GPIOB_ReadPortPin → 本平台 GPIO API(WCH 用此类,PY32 用 HAL_GPIO_*)
// 换 MCU 只改本 .c 实现,.h 接口不变,业务层零改动(见 bsp.md 平台隔离契约)

#include "xhh_BSP_Template.h"
#include "CONFIG.h"          // 厂商平台头(只在 .c 内 include,不进 .h,见 bsp.md)

// ===== 物理参数宏(只在 .c 内,业务层看不到,命名 APP_<类别>_<对象>_* 全大写) =====
#define APP_TEMPLATE_OBJ_A_PIN   GPIO_Pin_22
#define APP_TEMPLATE_OBJ_B_PIN   GPIO_Pin_4

// ===== BSP ≠ Task:不写 _Loop/_Cmd/使能位(见 bsp.md "BSP ≠ Task") =====
// BSP 是无状态服务层,只有 Init + 按类别的 Read/Write/Apply 等

/// @brief 初始化所有逻辑 ID 对应的硬件 GPIO
void xhh_BSP_Template_Init(void)
{
	GPIOB_ModeCfg(APP_TEMPLATE_OBJ_A_PIN, GPIO_ModeIN_PU);      // 上拉输入
	GPIOA_ModeCfg(APP_TEMPLATE_OBJ_B_PIN, GPIO_ModeOut_PP_5mA); // 推挽输出
}

/// @brief 读逻辑 ID 对应引脚电平
uint8_t xhh_BSP_Template_Read(xhh_BSP_TEMPLATE_ID_t id)
{
	switch (id)
	{
	case xhh_BSP_TEMPLATE_ID_OBJ_A:
		return (uint8_t)GPIOB_ReadPortPin(APP_TEMPLATE_OBJ_A_PIN);

	case xhh_BSP_TEMPLATE_ID_OBJ_B:
		return (uint8_t)GPIOA_ReadPortPin(APP_TEMPLATE_OBJ_B_PIN);

	default:
		return 0;
	}
}

/// @brief 写逻辑 ID 对应引脚电平
void xhh_BSP_Template_Write(xhh_BSP_TEMPLATE_ID_t id, uint8_t level)
{
	switch (id)
	{
	case xhh_BSP_TEMPLATE_ID_OBJ_A:
		// 输入脚不可写,按需改成输出或忽略
		break;

	case xhh_BSP_TEMPLATE_ID_OBJ_B:
		if (level)
			GPIOA_SetBits(APP_TEMPLATE_OBJ_B_PIN);
		else
			GPIOA_ResetBits(APP_TEMPLATE_OBJ_B_PIN);
		break;

	default:
		break;
	}
}
