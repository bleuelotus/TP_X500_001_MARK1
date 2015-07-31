#include "sensor.h"

//获得ADC值
//ch:通道值
u16 Get_Adc(u8 ch)
{
//设置指定ADC的规则组通道,一个序列,采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
}

//避障发射电路PWM发生器初始化,PC.9->TIM8_CH4
void SENSOR_PWM_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  				TIM_OCInitStructure;
 	GPIO_InitTypeDef 					GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);			//使能定时器8时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);	 	//使能PC端口时钟和AFIO复用功能时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 					//避障PWM发射端口-->PC.9 端口配置,TIM8_CH4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 			//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 					//根据设定参数初始化GPIOC.9

	//初始化TIM8
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	//初始化TIM8 Channel4 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC4Init(TIM8, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM8 OC4

	TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);  //使能TIM8在CCR4上的预装载寄存器

	TIM_Cmd(TIM8, ENABLE);  //使能TIM8
}

//传感器相关ADC端口初始化
void SENSOR_ADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div8);																																			//设置ADC分频因子8 72M/8=9M,ADC最大时间不能超过14M

	//PA3,PA4,PA5,PA6,PA7 作为模拟通道输入引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PC0,PC4,PC5 作为模拟通道输入引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

	ADC_ResetCalibration(ADC1);	//使能复位校准

	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束

	ADC_StartCalibration(ADC1);	 //开启AD校准

	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束

//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
}

//传感器相关外部中断端口初始化
void SENSOR_EXTI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);														//使能复用功能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);													//使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_1;																	//GPIOA.1->双轮抬起状态脚
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;															//上拉输入
	GPIO_Init(GPIOA,&GPIO_InitStructure);																					//GPIOA.1初始化

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);

	EXTI_InitStructure.EXTI_Line 		= EXTI_Line1;																	//GPIOA.1
  EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;												//配置为中断模式
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;												//中断下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;															//使能GPIOA.1所在的外部中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;									//抢占优先级1,
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;													//子优先级1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;																//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);
}

//按键初始化函数
void SENSOR_Init(void) //SENSOR相关IO初始化
{
 	GPIO_InitTypeDef 					GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE,ENABLE);//使能PROTA|PORTB|PORTC|PORTD|PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;//PA2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA2

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14|GPIO_Pin_15;//PB14~15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB14~15

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10|GPIO_Pin_12;//PC10~12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC10~12

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PD3~7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOD3~7

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//PE12~15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE12~15

	SENSOR_ADC_Init();																															//传感器相关ADC端口初始化

	SENSOR_PWM_Init(TIM8_PWM_ARR_Value,TIM8_PWM_PSC_Value);													//避障发射电路发射10Khz的PWM方波
	TIM_SetCompare4(TIM8,SENS_LED_Value_const);																			//避障发射电路PWM占空比
}

//SENSOR处理函数(不包含充电座定位红外接收头,红外接收头初始化放在remote.c文件中)
//返回对应SENSOR值
u16 SENSOR_Scan(u8 sensor_var)
{
//	u8 ADC_average_times = 10;
	u16 sensor_value;
	switch(sensor_var){
		case(SENS_F_L):																																//前避障左
			sensor_value = Get_Adc(SENS_F_L_ADC_CH);
		break;
		case(SENS_F_LM):																															//前避障左中
			sensor_value = Get_Adc(SENS_F_LM_ADC_CH);
		break;
		case(SENS_F_M):																																//前避障中
			sensor_value = Get_Adc(SENS_F_M_ADC_CH);
		break;
		case(SENS_F_RM):																															//前避障右中
			sensor_value = Get_Adc(SENS_F_RM_ADC_CH);
		break;
		case(SENS_F_R):																																//前避障右
			sensor_value = Get_Adc(SENS_F_R_ADC_CH);
		break;
		case(SENS_FI_L):																															//前避障碰撞左
			sensor_value = 0x0001&(SENS_FI_L_Value);
		break;
		case(SENS_FI_R):																															//前避障碰撞右
			sensor_value = 0x0001&(SENS_FI_R_Value);
		break;
		case(SENS_B_L):																																//探底检测左
			sensor_value = 0x0001&(SENS_B_L_Value);
		break;
		case(SENS_B_LM):																															//探底检测中左
			sensor_value = 0x0001&(SENS_B_LM_Value);
		break;
		case(SENS_B_RM):																															//探底检测中右
			sensor_value = 0x0001&(SENS_B_RM_Value);
		break;
		case(SENS_B_R):																																//探底检测右
			sensor_value = 0x0001&(SENS_B_R_Value);
		break;
		case(SENS_WHEEL_UP):																													//双轮抬起
			sensor_value = 0x0001&(SENS_WHEEL_UP_Value);
		break;
		case(SENS_NO_GARBAGE_BOX):																										//垃圾盒未放检测
			sensor_value = 0x0001&(SENS_NO_GARBAGE_BOX_Value);
		break;
		case(SENS_DUSH_FULL):																													//灰尘过多检测
			sensor_value = 0x0001&(SENS_DUSH_FULL_Value);
		break;
//		case(SENS_CHARGE_BOX_CON):																										//充电座连接检测
//			sensor_value = 0x0001&(SENS_CHARGE_BOX_CON_Value);
//		break;
//		case(SENS_CB_LOC_L):																													//充电座定位接收左
//			sensor_value = 0x0001&(SENS_CB_LOC_L_Value);
//		break;
//		case(SENS_CB_LOC_LM):																													//充电座定位中左
//			sensor_value = 0x0001&(SENS_CB_LOC_LM_Value);
//		break;
//		case(SENS_CB_LOC_RM):																													//充电座定位中右
//			sensor_value = 0x0001&(SENS_CB_LOC_RM_Value);
//		break;
//		case(SENS_CB_LOC_R):																													//充电座定位接收右
//			sensor_value = 0x0001&(SENS_CB_LOC_R_Value);
//		break;
		case(SENS_FAN_OVLOAD):																												//吸尘风机过载检测
			sensor_value = 0x0001&(SENS_FAN_OVLOAD_Value);
		break;
		case(SENS_M_SIDE_L_OVLOAD):																										//左边刷电机过载检测
			sensor_value = 0x0001&(SENS_M_SIDE_L_OVLOAD_Value);
		break;
		case(SENS_M_SIDE_R_OVLOAD):																										//右边刷电机过载检测
			sensor_value = 0x0001&(SENS_M_SIDE_R_OVLOAD_Value);
		break;
		case(SENS_M_MOVE_L_OVLOAD):																										//左行走电机过载检测
			sensor_value = 0x0001&(SENS_M_MOVE_L_OVLOAD_Value);
		break;
		case(SENS_M_MOVE_R_OVLOAD):																										//右行走电机过载检测
			sensor_value = 0x0001&(SENS_M_MOVE_R_OVLOAD_Value);
		break;
		case(SENS_Battery):																														//电池电量检测
			if(g_charge_start_flag)
				TIM_CtrlPWMOutputs(TIM1, DISABLE);																						//若充电开始，测量时先关闭PWM
			sensor_value = Get_Adc(SENS_Battery_ADC_CH);
			if(g_charge_start_flag)
				TIM_CtrlPWMOutputs(TIM1, ENABLE);																							//测量后打开PWM
		break;
		case(SENS_24V):																																//充电时电源电量检测
			sensor_value = Get_Adc(SENS_Charging_Battery_ADC_CH);
		break;
		case(SENS_Battery_temp):																											//电池温度检测
			sensor_value = Get_Adc(SENS_Battery_temp_ADC_CH);
		break;
		default:
			sensor_value = 0;
		break;
	}
	return(sensor_value);
}


////SENSOR处理函数(包含充电座定位红外接收头,直接读IO电平，不可用)
////返回对应SENSOR值
//u16 SENSOR_Scan(u8 sensor_var)
//{
////	u8 ADC_average_times = 10;
//	u16 sensor_value;
//	switch(sensor_var){
//		case(SENS_F_L):																																//前避障左
//			sensor_value = Get_Adc(SENS_F_L_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_L_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_L_Value);
//		break;
//		case(SENS_F_LM):																															//前避障左中
//			sensor_value = Get_Adc(SENS_F_LM_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_LM_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_LM_Value);
//		break;
//		case(SENS_F_M):																																//前避障中
//			sensor_value = Get_Adc(SENS_F_M_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_M_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_M_Value);
//		break;
//		case(SENS_F_RM):																															//前避障右中
//			sensor_value = Get_Adc(SENS_F_RM_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_RM_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_RM_Value);
//		break;
//		case(SENS_F_R):																																//前避障右
//			sensor_value = Get_Adc(SENS_F_R_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_R_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_R_Value);
//		break;
//		case(SENS_FI_L):																															//前避障碰撞左
//			sensor_value = 0x0001&(SENS_FI_L_Value);
//		break;
//		case(SENS_FI_R):																															//前避障碰撞右
//			sensor_value = 0x0001&(SENS_FI_R_Value);
//		break;
//		case(SENS_B_L):																																//探底检测左
//			sensor_value = 0x0001&(SENS_B_L_Value);
//		break;
//		case(SENS_B_LM):																															//探底检测中左
//			sensor_value = 0x0001&(SENS_B_LM_Value);
//		break;
//		case(SENS_B_RM):																															//探底检测中右
//			sensor_value = 0x0001&(SENS_B_RM_Value);
//		break;
//		case(SENS_B_R):																																//探底检测右
//			sensor_value = 0x0001&(SENS_B_R_Value);
//		break;
//		case(SENS_WHEEL_UP):																													//双轮抬起
//			sensor_value = 0x0001&(SENS_WHEEL_UP_Value);
//		break;
//		case(SENS_NO_GARBAGE_BOX):																										//垃圾盒未放检测
//			sensor_value = 0x0001&(SENS_NO_GARBAGE_BOX_Value);
//		break;
//		case(SENS_DUSH_FULL):																													//灰尘过多检测
//			sensor_value = 0x0001&(SENS_DUSH_FULL_Value);
//		break;
//		case(SENS_CHARGE_BOX_CON):																										//充电座连接检测
//			sensor_value = 0x0001&(SENS_CHARGE_BOX_CON_Value);
//		break;
//		case(SENS_CB_LOC_L):																													//充电座定位接收左
//			sensor_value = 0x0001&(SENS_CB_LOC_L_Value);
//		break;
//		case(SENS_CB_LOC_LM):																													//充电座定位中左
//			sensor_value = 0x0001&(SENS_CB_LOC_LM_Value);
//		break;
//		case(SENS_CB_LOC_RM):																													//充电座定位中右
//			sensor_value = 0x0001&(SENS_CB_LOC_RM_Value);
//		break;
//		case(SENS_CB_LOC_R):																													//充电座定位接收右
//			sensor_value = 0x0001&(SENS_CB_LOC_R_Value);
//		break;
//		case(SENS_FAN_OVLOAD):																												//吸尘风机过载检测
//			sensor_value = 0x0001&(SENS_FAN_OVLOAD_Value);
//		break;
//		case(SENS_M_SIDE_L_OVLOAD):																										//左边刷电机过载检测
//			sensor_value = 0x0001&(SENS_M_SIDE_L_OVLOAD_Value);
//		break;
//		case(SENS_M_SIDE_R_OVLOAD):																										//右边刷电机过载检测
//			sensor_value = 0x0001&(SENS_M_SIDE_R_OVLOAD_Value);
//		break;
//		case(SENS_M_MOVE_L_OVLOAD):																										//左行走电机过载检测
//			sensor_value = 0x0001&(SENS_M_MOVE_L_OVLOAD_Value);
//		break;
//		case(SENS_M_MOVE_R_OVLOAD):																										//右行走电机过载检测
//			sensor_value = 0x0001&(SENS_M_MOVE_R_OVLOAD_Value);
//		break;
//		case(SENS_Battery):																														//电池电量检测
//			sensor_value = Get_Adc(SENS_Battery_ADC_CH);
//		break;
//		case(SENS_24V):																																//充电时电源电量检测
//			sensor_value = Get_Adc(SENS_Charging_Battery_ADC_CH);
//		break;
//		case(SENS_Battery_temp):																											//电池温度检测
//			sensor_value = Get_Adc(SENS_Battery_temp_ADC_CH);
//		break;
//		default:
//			sensor_value = 0;
//		break;
//	}
//	return(sensor_value);
//}
