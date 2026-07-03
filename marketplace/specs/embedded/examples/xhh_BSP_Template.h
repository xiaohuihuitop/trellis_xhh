// ===== 项目占位（复制后必须替换/确认已定义）=====
// 本骨架以 GPIO 类别为示例。复制后:
//   1. 文件名 xhh_BSP_Template.c/.h → xhh_BSP_GPIO.c/.h(或其他基础能力类别 PWM/ADC/Flash/RTC/Power/System)
//   2. 全局替换 Template/TEMPLATE → 类别名(如 GPIO/GPIO,缩写词全大写)
//   3. 在 xhh_BSP_TEMPLATE_ID_t 枚举里填本项目真实用的逻辑 ID 对象名
//   4. 在 .c 内 APP_TEMPLATE_*_PIN 物理宏填真实引脚(查芯片数据手册)
//   5. 注册到 xhh_BSP_Def.h(若需跨类别共用类型)
// 依赖:xhh_BSP_Def.h(已存在则 include,见 bsp.md)
// 禁止:新增设备型 BSP(xhh_BSP_Key/xhh_BSP_Motor 等),见 bsp.md 黑名单

#ifndef __XHH_BSP_TEMPLATE_H__
#define __XHH_BSP_TEMPLATE_H__

#include "xhh_BSP_Def.h"

// ===== 逻辑 ID 枚举(业务层只认这个,看不到物理引脚) =====
typedef enum
{
	xhh_BSP_TEMPLATE_ID_Null = 0,
	xhh_BSP_TEMPLATE_ID_OBJ_A,        // TODO: 换成真实逻辑对象名(如 KEY_MATCH)
	xhh_BSP_TEMPLATE_ID_OBJ_B,        // TODO: 同上
} xhh_BSP_TEMPLATE_ID_t;

// ===== 公开接口(命名 xhh_BSP_<类别>_<动词>,缩写词全大写) =====
/// @brief 初始化本类别所有逻辑 ID 对应的硬件
void xhh_BSP_Template_Init(void);

/// @brief 读逻辑 ID 对应的引脚电平
/// @param id 逻辑 ID
/// @return 0/1
uint8_t xhh_BSP_Template_Read(xhh_BSP_TEMPLATE_ID_t id);

/// @brief 写逻辑 ID 对应的引脚电平
/// @param id 逻辑 ID
/// @param level 0/1
void xhh_BSP_Template_Write(xhh_BSP_TEMPLATE_ID_t id, uint8_t level);

#endif
