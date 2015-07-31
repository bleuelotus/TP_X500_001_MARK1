#ifndef __CHARGER_H
#define __CHARGER_H

#include "usr_includes.h"

void CHARGER_Init(void);																											//初始化充电接口
void CHARGER_Start(void);																											//充电开始配置
void CHARGER_Stop(void);																											//充电停止配置
void CHARGER_Start_1d4(u8 charge_pwm_pulse_width_tmp);												//电池电量低于1/4满电电量时的充电控制函数
void CHARGER_Start_more_than_1d4(void);	 																			//电池电量高于1/4满电电量时,充电控制端输出占空比设置为45.5%
void CHARGER_Startt_more_than_3d4(void);																			//电池电量高于3/4满电电量时,充电控制端输出占空比设置为60%

#endif
