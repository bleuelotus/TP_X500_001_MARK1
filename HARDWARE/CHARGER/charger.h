#ifndef __CHARGER_H
#define __CHARGER_H

#include "usr_includes.h"

void CHARGER_Init(void);																											//��ʼ�����ӿ�
void CHARGER_Start(void);																											//��翪ʼ����
void CHARGER_Stop(void);																											//���ֹͣ����
void CHARGER_Start_1d4(u8 charge_pwm_pulse_width_tmp);												//��ص�������1/4�������ʱ�ĳ����ƺ���
void CHARGER_Start_more_than_1d4(void);	 																			//��ص�������1/4�������ʱ,�����ƶ����ռ�ձ�����Ϊ45.5%
void CHARGER_Startt_more_than_3d4(void);																			//��ص�������3/4�������ʱ,�����ƶ����ռ�ձ�����Ϊ60%

#endif
