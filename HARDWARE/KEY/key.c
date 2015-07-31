#include "key.h"

////按键初始化函数
//void KEY_Init(void) //IO初始化SPOT-->GPIOB.5,CLEAN-->GPIOB.6,PLAN-->GPIOB.7,HOME-->GPIOB.8 上拉输入
//{
// 	GPIO_InitTypeDef GPIO_InitStructure;

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB时钟

//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;//PB5~8
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB5,6,7,8
//}

void KEY_Init(void)	//IO初始化SPOT-->GPIOB.5,CLEAN-->GPIOB.6,PLAN-->GPIOB.7,HOME-->GPIOB.8 上拉输入,中断方式
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);						//使能复用功能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);					//使能GPIOB时钟
	
	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

//GPIOB 中断线以及中断初始化配置   下降沿触发
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource5);

  EXTI_InitStructure.EXTI_Line 		= EXTI_Line5;																											//GPIOB.5
  EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;																						//配置为中断模式
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;																						//中断下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 																																	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource6);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line6;	
	EXTI_Init(&EXTI_InitStructure);	 
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource7);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line7;	
	EXTI_Init(&EXTI_InitStructure);	 
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line8;	
	EXTI_Init(&EXTI_InitStructure);	 

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;																								//使能GPIOB.5|GPIOB.6|GPIOB.7|GPIOB.8所在的外部中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;																			//抢占优先级2,
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;																							//子优先级1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;																										//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);
}

////按键处理函数(中断方式)
//u8 KEY_Scan(void)
//{
//	if(EXTI_Key_flag)
//	{
//		EXTI_Key_flag = 0;
//		return KeySta;
//	}
// 	return 0;// 无按键按下
//}

////按键处理函数(轮询方式)
////返回按键值
////mode:0,不支持连续按;1,支持连续按;
////0,没有任何按键按下
////1,KEY_SPOT按下
////2,KEY_CLEAN按下
////3,KEY_PLAN按下
////4,KEY_HOME按下
////注意此函数有响应优先级,KEY0>KEY1>KEY2>KEY3!!
//u8 KEY_Scan(u8 mode)
//{
//	static u8 key_up=1;																	//按键按松开标志
//	if(mode)	key_up=1;  																//支持连按
//	
//	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==0))
//	{
//		key_up=0;
//		if(KEY0==0)return KEY_SPOT;
//		else if(KEY1==0)return KEY_CLEAN;
//		else if(KEY2==0)return KEY_PLAN;
//		else if(KEY3==0)return KEY_HOME;
//	}else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1)key_up=1;
// 	return 0;// 无按键按下
//}
