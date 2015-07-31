#ifndef __POWER5V_H
#define __POWER5V_H

#include "usr_includes.h"

#define POWER5V_VOUT5V_CTRL				PEout(6)			//VOUT5V���ƶ˿ڿ��ƺ�,0�Ͽ�,1����
#define POWER5V_V78L05_CTRL				PCout(2)			//V78L05���ƶ˿ڿ��ƺ�,0�Ͽ�,1����

void POWER5V_Init(void);									//��ʼ��VOUT 5V���ƶ˿�
void POWER5V_on(void);										//VOUT 5V���
void POWER5V_off(void);										//VOUT 5V�ж�

#endif
