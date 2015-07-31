#include "power5v.h"

//5V��Դ��ʼ��
void POWER5V_Init(void)																															
{
	GPIO_InitTypeDef  				GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE, ENABLE);	 		//ʹ��PC��PE�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6;				 																//CHARGER_EN-->PE.6�˿�����
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_OD; 		 														//��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 															//IO���ٶ�Ϊ2MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 																	//�����趨������ʼ��GPIOE.6
	GPIO_ResetBits(GPIOE, GPIO_Pin_6);						 																		//PE.6 �����

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;				 																//CHARGER_EN-->PC.2�˿�����
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP; 		 														//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 															//IO���ٶ�Ϊ2MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 																	//�����趨������ʼ��GPIOC.2
	GPIO_ResetBits(GPIOC, GPIO_Pin_2);						 																		//PC.2 �����
	
	POWER5V_on();
}

//5V��Դ����
void POWER5V_on(void)
{
	POWER5V_VOUT5V_CTRL = 1;
	POWER5V_V78L05_CTRL = 1;
}

//5V��Դ�ر�
void POWER5V_off(void)
{
	POWER5V_VOUT5V_CTRL = 0;
	POWER5V_V78L05_CTRL = 0;
}
