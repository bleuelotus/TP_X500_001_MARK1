#ifndef __MOTOR_FAN_H
#define __MOTOR_FAN_H

#include "usr_includes.h"

void MOTOR_FAN_Init(void);													//初始化吸尘风机
void MOTOR_FAN(u8 m_fan_run,u16 m_fan_speed); 			//风机启停控制,cmd->1开启,0停止；FAN_speed->TIM8_CH1调整风机转速
void MOTOR_FAN_SOFT_START(u8 m_fan_run, u16 m_fan_speed_max, u16 m_fan_step, u8 timer_flag);	//机器人风机软启动函数

#endif
