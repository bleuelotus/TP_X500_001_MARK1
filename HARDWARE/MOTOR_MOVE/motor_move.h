#ifndef __MOTOR_MOVE_H
#define __MOTOR_MOVE_H

#include "usr_includes.h"

void 	MOTOR_MOVE_Init(void);																				//初始化行走电机
void 	MOTOR_MOVE_PWM_Init(u16 arr, u16 psc);												//初始化行走电机PWM输出

//void MOTOR_MOVE_SOFT_START(u8 m_motor_run, u16 m_motor_speed_max, u16 m_motor_time);					//机器人行走电机软启动函数
void 	MOTOR_MOVE_SOFT_START_SIMPLE(void);																											//机器人行走电机软启动函数(简单加速)

void 	MOTOR_MOVE_FORWARD(void);																			//机器人前进_0
void 	MOTOR_MOVE_BACKWARD(void);																		//机器人后退_1
void 	MOTOR_MOVE_CLOCKWIZE(void);																		//机器人顺时针旋转_2
void 	MOTOR_MOVE_ANTICLOCKWIZE(void);	 															//机器人逆时针旋转_3
void 	MOTOR_MOVE_TURNLEFT(u8 m_move_speed);													//电机左转_4
void 	MOTOR_MOVE_TURNRIGHT(u8 m_move_speed);												//电机右转_5
void 	MOTOR_MOVE_BACKLEFT(u8 m_move_speed);													//机器人左后退_6
void 	MOTOR_MOVE_BACKRIGHT(u8 m_move_speed);												//机器人右后退_7
void 	MOTOR_MOVE_STOP(void);																				//机器人停止_8	
void 	MOTOR_MOVE_RUN(u8 m_move_speed);															//机器人运行_9

u16 	MOTOR_MOVE_DIST_L_READ(void);																	//左行走电机里程计数值读取函数
u16 	MOTOR_MOVE_DIST_R_READ(void);																	//右行走电机里程计数值读取函数

void	MOTOR_MOVE(u8 m_move_direc, u16 m_move_speed);																								//机器人运动控制函数;run_flag->0运行,1停止;direc->0前进,1后退,2前进,3后退;speed->速度
void 	MOTOR_TURN_Angle(u16 m_move_angle, u16 m_move_direc_flag, u16 m_move_speed);																//控制行走电机转向m_move_angle度的函数
void 	MOTOR_MOVE_AVOID(u16 m_move_backward_angle, u16 m_move_turn_angle, u16 m_move_direc, u16 m_move_speed);			//行走电机避障动作函数,后退->转向

#define M_MOVE_L	1
#define M_MOVE_R	0

#endif
