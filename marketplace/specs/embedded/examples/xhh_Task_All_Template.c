#include "xhh_Task_All_Template.h"

void xhh_Task_ALL_Cmd(uint8_t en)
{
	xhh_Task_ADC_Cmd(en);
	xhh_Task_Flash_Cmd(en);
	xhh_Task_Key_Cmd(en);
	xhh_Task_Motor_Cmd(en);
}
