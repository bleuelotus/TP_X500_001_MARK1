#ifndef __EXTI_H
#define __EXIT_H

#include "usr_includes.h"

#define RDATA PDin(11)	 //红外数据输入脚

#define REMOTE_ID 0			//红外遥控识别码(ID)

extern u8 RmtCnt;	//按键按下的次数

void REMOTE_Init(void);//红外遥控接收端口初始化
u8 REMOTE_Scan(void);

void RETURN_Init(void);//自动返回红外接收端口初始化
u8 RETURN_Scan(void);

#endif

