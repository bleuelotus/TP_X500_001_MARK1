#include "charger.h"

//开启充电PWM输出
void CHARGER_Start(void)																																					
{
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
//	TIM1->CCER |= (uint16_t)((uint16_t)TIM_CCER_CC1E);
}

//关闭充电PWM输出
void CHARGER_Stop(void)																																						
{
	TIM_CtrlPWMOutputs(TIM1, DISABLE);
//	TIM1->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC1E);
}

//充电通断控制端口PWM配置
void CHARGER_PWM_Init(u16 arr, u16 psc)																														
{
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  				TIM_OCInitStructure;
	GPIO_InitTypeDef  				GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_TIM1|RCC_APB2Periph_AFIO, ENABLE);		//使能PA端口时钟和TIM1时钟和AFIO复用功能时钟

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_8;																											//CHARGER_CUTOFF-->PA.8 端口配置
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;																								//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);

	 //初始化TIM1
 TIM_TimeBaseStructure.TIM_Period = arr; 																													//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
 TIM_TimeBaseStructure.TIM_Prescaler = psc; 																											//设置用来作为TIMx时钟频率除数的预分频值
 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 																					//设置时钟分割:TDTS = Tck_tim
 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  																		//TIM向上计数模式
 TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); 																									//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

 //初始化TIM8 Channel2,Channel3 PWM模式
 TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 																								//选择定时器模式:TIM脉冲宽度调制模式1
 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 																		//比较输出使能
 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 																				//输出极性:TIM输出比较极性高
 TIM_OC1Init(TIM1, &TIM_OCInitStructure);  											 																	//根据指定的参数初始化外设TIM1 OC1

 TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  																							//使能TIM1在CCR1上的预装载寄存器

 TIM_Cmd(TIM1, ENABLE);																																						//使能TIM1
}

//充电接口初始化
//实测输出PWM为30khz、60%占空比,当电池电压低于1/4电压时,采用13%~60%,步进1.5%占空比的方式充电
void CHARGER_Init(void)																												
{
	CHARGER_PWM_Init(33,71);																											//初始化TIM1_CH1->PA8,PWM频率30Khz
	CHARGER_Stop();																															//禁能充电PWM输出
	TIM_SetCompare1(TIM1, 15);																									//占空比设置为45.5%,按锂电池14.4V电压情况下推算出的充电电压,充电电压=锂电池额定电压*1.15=14.4*1.15=16.56V,16.56/24=0.69																			
}

//电池电量低于1/4满电电量时的充电控制函数
void CHARGER_Start_1d4(u8 charge_pwm_pulse_width_tmp)
{
	if(charge_pwm_pulse_width_tmp < 10){
		charge_pwm_pulse_width_tmp++;
		TIM_SetCompare1(TIM1, charge_pwm_pulse_width_tmp);
	}else{
		charge_pwm_pulse_width_tmp = 4;
		TIM_SetCompare1(TIM1, charge_pwm_pulse_width_tmp);
	}
	CHARGER_Start();
}

//电池电量高于1/4满电电量时,充电控制端输出占空比设置为45.5%
void CHARGER_Start_more_than_1d4(void)
{
	TIM_SetCompare1(TIM1, 15);
	CHARGER_Start();
}

//电池电量高于3/4满电电量时，充电控制端输出占空比设置为15%,涓流充电
void CHARGER_Startt_more_than_3d4(void)
{
	TIM_SetCompare1(TIM1, 5);
	CHARGER_Start();
}
