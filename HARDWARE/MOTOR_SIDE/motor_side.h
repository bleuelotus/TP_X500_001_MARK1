#ifndef __MOTOR_SIDE_H
#define __MOTOR_SIDE_H	 

#include "usr_includes.h"

void MOTOR_SIDE_Init(void);																//初始化边刷电机
void MOTOR_SIDE(u8 m_side_run);														//边刷电机启停控制,m_side_run->1运行,0停止
		 				    
#endif
