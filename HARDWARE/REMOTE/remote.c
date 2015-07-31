#include "remote.h"

//遥控器接收状态
//[7]:收到了引导码标志
//[6]:得到了一个按键的所有信息
//[5]:保留
//[4]:标记上升沿是否已经被捕获
//[3:0]:溢出计时器
u8 	RmtSta=0;
u16 Dval;				//下降沿时计数器的值
u32 RmtRec=0;		//红外接收到的数据
u8  RmtCnt=0;		//按键按下的次数
u8	RmtRec_Cnt = 0;	//用于记录接收到的数据的位数以便判断是否接收完成
u8 	EXTI_Rmt_flag;

//自动返回充电座红外接收状态
//[7]:收到了引导码标志
//[6]:得到了一个按键的所有信息
//[5]:保留
//[4]:标记上升沿是否已经被捕获
//[3:0]:溢出计时器
u8 	Rtrn_RmtSta=0;
u16 Rtrn_Dval;						//下降沿时计数器的值
u32 Rtrn_RmtRec=0;				//红外接收到的数据
u8  Rtrn_RmtCnt=0;				//按键按下的次数
u8	Rtrn_RmtRec_Cnt = 0;	//用于记录接收到的数据的位数以便判断是否接收完成
u8 	Rtrn_EXTI_Rmt_flag;		


//通用定时器2中断初始化,用于统计红外接收中断触发后的时间
//这里时钟选择为APB1的2倍,而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
void TIM2_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能

	//定时器TIM5初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位

	TIM_Cmd(TIM2, ENABLE);  //使能TIM2
}

//红外接收端口初始化
void REMOTE_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM2_Int_Init(65535, 71);																			//初始化定时器2作为红外接收触发计时器,每1usTIM2计数器加1

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);						//使能复用功能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);					//使能GPIOD时钟

//GPIOD.11 中断线以及中断初始化配置   上升沿触发
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource11);

	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;
	GPIO_Init(GPIOD,&GPIO_InitStructure);

  EXTI_InitStructure.EXTI_Line = EXTI_Line11;										//GPIOD.11
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;						//配置为中断模式
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;				//中断上升沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 															//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;					//使能GPIOD.11所在的外部中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2,
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);
}

//自动返回红外接收端口初始化
void RETURN_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);						//使能复用功能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);					//使能GPIOD时钟

	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	
	//GPIOD.12中断线以及中断初始化配置   上升沿触发
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource12);

  EXTI_InitStructure.EXTI_Line = EXTI_Line12;										//GPIOD.12
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;						//配置为中断模式
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;				//中断上升沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 															//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
	
	//GPIOD.13中断线以及中断初始化配置   上升沿触发
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource13);
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;										//GPIOD.13
	EXTI_Init(&EXTI_InitStructure);

	//GPIOD.14中断线以及中断初始化配置   上升沿触发
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource14);
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;										//GPIOD.14
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOD.15中断线以及中断初始化配置   上升沿触发
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource15);
	EXTI_InitStructure.EXTI_Line = EXTI_Line15;										//GPIOD.15
	EXTI_Init(&EXTI_InitStructure);
}

//红外接收中断处理
void EXTI15_10_IRQHandler(void)
{
//	EXTI->IMR &= ~(1<<11); 																		//屏蔽来自LINE11(GPIOD.11)上的中断请求

 if(EXTI_GetFlagStatus(EXTI_Line11)!=RESET){
		if(RDATA){																							//上升沿捕获
	    TIM_SetCounter(TIM2,0);	   															//清空定时器计数值
			TIM_Cmd(TIM2, ENABLE);																	//使能定时器2开始计时
			EXTI->RTSR &= ~(1<<11);																	//禁止红外接收中断上升沿触发模式
			EXTI->FTSR |= (1<<11);																	//设置红外接收中断为下降沿触发模式
			RmtSta|=0x10;																						//标记上升沿已经被捕获
		}else{ 																									//下降沿捕获
			Dval = TIM2->CNT;																				//读取TIM2计数器值
			TIM_Cmd(TIM2, DISABLE);																	//停止定时器2计时
			EXTI->FTSR &= ~(1<<11);																	//禁止红外接收中断下降沿触发模式
			EXTI->RTSR |= (1<<11);																	//设置红外接收中断为上升沿触发模式

			if(RmtSta&0x10){																				//完成一次高电平捕获
 				if(RmtSta&0x80){																			//接收到了引导码
					if(Dval>300&&Dval<800){																//560为标准值,560us
						RmtRec<<=1;																						//左移一位.
						RmtRec|=0;																							//接收到0
						RmtRec_Cnt++;																						//RmtRec接收到的数据位数增加一位
					}else if(Dval>1400&&Dval<1800){												//1680为标准值,1680us
						RmtRec<<=1;																							//左移一位.
						RmtRec|=1;																							//接收到1
						RmtRec_Cnt++;																						//RmtRec接收到的数据位数增加一位
					}else if(Dval>2200&&Dval<2600){												//得到按键键值增加的信息 2500为标准值2.5ms
						RmtCnt++; 																							//按键次数增加1次
						RmtSta&=0xF0;																						//清空计时器
					}
 				}else if(Dval>4200&&Dval<4700){													//4500为标准值4.5ms
					RmtSta|=1<<7;																						//标记成功接收到了引导码
					RmtCnt=0;																								//清除按键次数计数器
				}
			}
			RmtSta&=~(1<<4);
		}
	}
	if(RmtRec_Cnt == 32){
		if(EXTI_GetFlagStatus(EXTI_Line11)!=RESET){
			if(RmtSta&0x80){															//上次有数据被接收到了
				RmtSta&=~0x10;															//取消上升沿已经被捕获标记
				if((RmtSta&0x0F)==0x00)
					RmtSta|=1<<6;															//标记已经完成一次按键的键值信息采集
				if((RmtSta&0x0F)<14)
					RmtSta++;																	//溢出计时器++
				else{
					RmtSta&=~(1<<7);													//清空引导标识
					RmtSta&=0xF0;															//清空计数器
				}
			}
		}
		RmtRec_Cnt = 0;
		EXTI_Rmt_flag = 1;
	}
//	EXTI->IMR |= (1<<11);																				//使能GPIOD.11中断线
	EXTI->PR  |= (1<<11);																				//清除LINE11上的中断标识
}

//处理红外键盘
//返回值:
//	 0,没有任何按键按下
//其他,按下的按键键值.
u8 REMOTE_Scan(void)
{
	u8 sta=0;
  u8 t1,t2;
	if(EXTI_Rmt_flag){
		if(RmtSta&(1<<6)){//得到一个按键的所有信息了
			t1=RmtRec>>24;			//得到地址码
			t2=(RmtRec>>16)&0xff;	//得到地址反码
			if((t1==(u8)~t2)&&t1==REMOTE_ID){//检验遥控识别码(ID)及地址
				t1=RmtRec>>8;
				t2=RmtRec;
				if(t1==(u8)~t2)sta=t1;//键值正确
			}
			if((sta==0)||((RmtSta&0x80)==0)){//按键数据错误/遥控已经没有按下了
				RmtSta&=~(1<<6);//清除接收到有效按键标识
				RmtCnt=0;		//清除按键次数计数器
			}
		}
		EXTI_Rmt_flag = 0;
	}else
//		if(RmtCnt);								//如果RmtCnt大于0,则说明按键重复按下,sta保留上次读取值
//		else{
			sta = 0;
//		}
//	if(EXTI_Rmt_flag){
//		printf("RmtRec_Cnt is:%d\r\n",RmtRec_Cnt);
//		printf("sta is:%d\r\n",sta);
//		exti_flag = 0;
//	}
  return sta;
}

//自动返回充电座接收信号处理
//返回值:
//	 0,没有任何按键按下
//其他,按下的按键键值.
u8 RETURN_Scan(void)
{
	u8 sta=0;
  u8 t1,t2;
	if(Rtrn_EXTI_Rmt_flag){
		if(Rtrn_RmtSta&(1<<6)){//得到一个按键的所有信息了
			t1=Rtrn_RmtRec>>24;			//得到地址码
			t2=(Rtrn_RmtRec>>16)&0xff;	//得到地址反码
			if((t1==(u8)~t2)&&t1==REMOTE_ID){//检验遥控识别码(ID)及地址
				t1=Rtrn_RmtRec>>8;
				t2=Rtrn_RmtRec;
				if(t1==(u8)~t2)sta=t1;//键值正确
			}
			if((sta==0)||((Rtrn_RmtSta&0x80)==0)){//按键数据错误/遥控已经没有按下了
				Rtrn_RmtSta&=~(1<<6);//清除接收到有效按键标识
				RmtCnt=0;		//清除按键次数计数器
			}
		}
		Rtrn_EXTI_Rmt_flag = 0;
	}else
//		if(RmtCnt);								//如果RmtCnt大于0,则说明按键重复按下,sta保留上次读取值
//		else{
			sta = 0;
//		}
//	if(EXTI_Rmt_flag){
//		printf("RmtRec_Cnt is:%d\r\n",RmtRec_Cnt);
//		printf("sta is:%d\r\n",sta);
//		exti_flag = 0;
//	}
	
  return sta;
}
