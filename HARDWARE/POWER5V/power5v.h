#ifndef __POWER5V_H
#define __POWER5V_H

#include "usr_includes.h"

#define POWER5V_VOUT5V_CTRL				PEout(6)			//VOUT5V控制端口控制宏,0断开,1开启
#define POWER5V_V78L05_CTRL				PCout(2)			//V78L05控制端口控制宏,0断开,1开启

void POWER5V_Init(void);									//初始化VOUT 5V控制端口
void POWER5V_on(void);										//VOUT 5V输出
void POWER5V_off(void);										//VOUT 5V切断

#endif
