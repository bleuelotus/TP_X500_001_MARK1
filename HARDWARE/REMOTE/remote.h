#ifndef __EXTI_H
#define __EXIT_H

#include "usr_includes.h"

#define RDATA PDin(11)	 //�������������

#define REMOTE_ID 0			//����ң��ʶ����(ID)

extern u8 RmtCnt;	//�������µĴ���

void REMOTE_Init(void);//����ң�ؽ��ն˿ڳ�ʼ��
u8 REMOTE_Scan(void);

void RETURN_Init(void);//�Զ����غ�����ն˿ڳ�ʼ��
u8 RETURN_Scan(void);

#endif

