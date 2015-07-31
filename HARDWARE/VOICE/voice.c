#include "voice.h"

//������оƬдָ��
void VOICE_WR(u8 cmd)																													
{
	u8 i;
	VOICE_RST = 0;
	delay_ms(5);
	VOICE_RST = 1;
	delay_ms(60);
	VOICE_DIN = 0;
	delay_ms(5);

	for(i=0;i<8;i++){
		if(cmd&0x01){
			VOICE_DIN = 1;
			delay_us(900);
			VOICE_DIN = 0;
			delay_us(300);
		}else{
			VOICE_DIN = 1;
			delay_us(300);
			VOICE_DIN = 0;
			delay_us(900);
		}
		cmd = cmd>>1;
	}
		VOICE_DIN = 1;
}

//��ʼ������оƬ
void VOICE_Init(void)																													
{
 GPIO_InitTypeDef  GPIO_InitStructure;

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 											//ʹ��PE�˿�ʱ��

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 														//VOICE_RSTn-->PE.3 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 												//�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 												//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 															//�����趨������ʼ��GPIOE.3
 GPIO_SetBits(GPIOE,GPIO_Pin_3);						 																	//PE.3 �����

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	    		 													//VOICE_P03-->PE.4 �˿�����
 GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 														//������� ,IO���ٶ�Ϊ50MHz
 GPIO_SetBits(GPIOE,GPIO_Pin_4); 						 			 														//PE.4 �����

 VOICE_WR(0xE3);																															//��������оƬ����,E0H��С,EFH���
 delay_ms(20);
 VOICE_WR(0xF1);																															//������оƬ�ڲ�����
}

//����adr��Ӧ��ַ�ε�����,adr��Χ��00H~CFH
void VOICE_PLAY(u8 adr)																												
{
	VOICE_WR(0xF3);
	VOICE_WR(adr);
//	VOICE_WR(0xF8);																															//�������������1s�ľ���ʱ��
//	VOICE_WR(10);
}
