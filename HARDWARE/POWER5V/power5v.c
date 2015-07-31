#include "power5v.h"

//5V电源初始化
void POWER5V_Init(void)																															
{
	GPIO_InitTypeDef  				GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE, ENABLE);	 		//使能PC、PE端口时钟

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6;				 																//CHARGER_EN-->PE.6端口配置
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_OD; 		 														//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 															//IO口速度为2MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 																	//根据设定参数初始化GPIOE.6
	GPIO_ResetBits(GPIOE, GPIO_Pin_6);						 																		//PE.6 输出低

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;				 																//CHARGER_EN-->PC.2端口配置
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP; 		 														//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 															//IO口速度为2MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 																	//根据设定参数初始化GPIOC.2
	GPIO_ResetBits(GPIOC, GPIO_Pin_2);						 																		//PC.2 输出低
	
	POWER5V_on();
}

//5V电源开启
void POWER5V_on(void)
{
	POWER5V_VOUT5V_CTRL = 1;
	POWER5V_V78L05_CTRL = 1;
}

//5V电源关闭
void POWER5V_off(void)
{
	POWER5V_VOUT5V_CTRL = 0;
	POWER5V_V78L05_CTRL = 0;
}
