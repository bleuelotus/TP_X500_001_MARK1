#include "motor_side.h"

//��ˢ������ƶ˿ڳ�ʼ������
void MOTOR_SIDE_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 	//ʹ��PE�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 				//���ˢ������ƶ˿�-->PE.5 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 					//�����趨������ʼ��GPIOE.5
 GPIO_ResetBits(GPIOE,GPIO_Pin_5);						 						//PE.5 �����
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 				//�ұ�ˢ������ƶ˿�-->PE.11 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 					//�����趨������ʼ��GPIOE.11
 GPIO_ResetBits(GPIOE,GPIO_Pin_11);						 						//PE.11 �����
}

void MOTOR_SIDE(u8 m_side_run)														//��ˢ�����ͣ����,m_side_run->1����,0ֹͣ
{
	if(m_side_run){
		M_SIDE_L = M_SIDE_Run;
		M_SIDE_R = M_SIDE_Run;
	}else{
		M_SIDE_L = M_SIDE_Stop;
		M_SIDE_R = M_SIDE_Stop;
	}
}
 
