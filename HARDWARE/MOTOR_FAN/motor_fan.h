#ifndef __MOTOR_FAN_H
#define __MOTOR_FAN_H

#include "usr_includes.h"

void MOTOR_FAN_Init(void);													//��ʼ���������
void MOTOR_FAN(u8 m_fan_run,u16 m_fan_speed); 			//�����ͣ����,cmd->1����,0ֹͣ��FAN_speed->TIM8_CH1�������ת��
void MOTOR_FAN_SOFT_START(u8 m_fan_run, u16 m_fan_speed_max, u16 m_fan_step, u8 timer_flag);	//�����˷������������

#endif
