#include "voice.h"

//向语音芯片写指令
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

//初始化语音芯片
void VOICE_Init(void)																													
{
 GPIO_InitTypeDef  GPIO_InitStructure;

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 											//使能PE端口时钟

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 														//VOICE_RSTn-->PE.3 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 												//推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 												//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 															//根据设定参数初始化GPIOE.3
 GPIO_SetBits(GPIOE,GPIO_Pin_3);						 																	//PE.3 输出高

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	    		 													//VOICE_P03-->PE.4 端口配置
 GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 														//推挽输出 ,IO口速度为50MHz
 GPIO_SetBits(GPIOE,GPIO_Pin_4); 						 			 														//PE.4 输出高

 VOICE_WR(0xE3);																															//设置语音芯片音量,E0H最小,EFH最大
 delay_ms(20);
 VOICE_WR(0xF1);																															//打开语音芯片内部功放
}

//播放adr对应地址段的语音,adr范围：00H~CFH
void VOICE_PLAY(u8 adr)																												
{
	VOICE_WR(0xF3);
	VOICE_WR(adr);
//	VOICE_WR(0xF8);																															//播放语音后插入1s的静音时间
//	VOICE_WR(10);
}
