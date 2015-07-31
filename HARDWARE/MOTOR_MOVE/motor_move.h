#ifndef __MOTOR_MOVE_H
#define __MOTOR_MOVE_H

#include "usr_includes.h"

void 	MOTOR_MOVE_Init(void);																				//��ʼ�����ߵ��
void 	MOTOR_MOVE_PWM_Init(u16 arr, u16 psc);												//��ʼ�����ߵ��PWM���

//void MOTOR_MOVE_SOFT_START(u8 m_motor_run, u16 m_motor_speed_max, u16 m_motor_time);					//���������ߵ������������
void 	MOTOR_MOVE_SOFT_START_SIMPLE(void);																											//���������ߵ������������(�򵥼���)

void 	MOTOR_MOVE_FORWARD(void);																			//������ǰ��_0
void 	MOTOR_MOVE_BACKWARD(void);																		//�����˺���_1
void 	MOTOR_MOVE_CLOCKWIZE(void);																		//������˳ʱ����ת_2
void 	MOTOR_MOVE_ANTICLOCKWIZE(void);	 															//��������ʱ����ת_3
void 	MOTOR_MOVE_TURNLEFT(u8 m_move_speed);													//�����ת_4
void 	MOTOR_MOVE_TURNRIGHT(u8 m_move_speed);												//�����ת_5
void 	MOTOR_MOVE_BACKLEFT(u8 m_move_speed);													//�����������_6
void 	MOTOR_MOVE_BACKRIGHT(u8 m_move_speed);												//�������Һ���_7
void 	MOTOR_MOVE_STOP(void);																				//������ֹͣ_8	
void 	MOTOR_MOVE_RUN(u8 m_move_speed);															//����������_9

u16 	MOTOR_MOVE_DIST_L_READ(void);																	//�����ߵ����̼���ֵ��ȡ����
u16 	MOTOR_MOVE_DIST_R_READ(void);																	//�����ߵ����̼���ֵ��ȡ����

void	MOTOR_MOVE(u8 m_move_direc, u16 m_move_speed);																								//�������˶����ƺ���;run_flag->0����,1ֹͣ;direc->0ǰ��,1����,2ǰ��,3����;speed->�ٶ�
void 	MOTOR_TURN_Angle(u16 m_move_angle, u16 m_move_direc_flag, u16 m_move_speed);																//�������ߵ��ת��m_move_angle�ȵĺ���
void 	MOTOR_MOVE_AVOID(u16 m_move_backward_angle, u16 m_move_turn_angle, u16 m_move_direc, u16 m_move_speed);			//���ߵ�����϶�������,����->ת��

#define M_MOVE_L	1
#define M_MOVE_R	0

#endif
