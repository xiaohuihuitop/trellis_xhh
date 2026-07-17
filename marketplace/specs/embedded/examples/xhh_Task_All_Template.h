#ifndef XHH_TASK_ALL_H
#define XHH_TASK_ALL_H

#include "xhh_Task_ADC.h"
#include "xhh_Task_Flash.h"
#include "xhh_Task_Key.h"
#include "xhh_Task_Motor.h"

/**
 * @brief AI:统一使能或关闭需要参与聚合控制的 Task。
 * @param en 0 全部关闭，非 0 全部使能。
 * @return 无。
 */
void xhh_Task_ALL_Cmd(uint8_t en);

#endif
