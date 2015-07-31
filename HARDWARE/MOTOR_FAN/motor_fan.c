#include "motor_fan.h"

void MOTOR_FAN_Init(void)
{
 GPIO_InitTypeDef 				GPIO_InitStructure;
 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 TIM_OCInitTypeDef  			TIM_OCInitStructure;

 RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);			//使能定时器8时钟
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 	//使能PC端口时钟

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				 				//吸尘风机控制端口-->PC.6 端口配置,TIM8_CH1
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 			//复用推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 					//根据设定参数初始化GPIOC.6

	//初始化TIM8
	TIM_TimeBaseStructure.TIM_Period = TIM8_PWM_ARR_Value; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值,1Khz
	TIM_TimeBaseStructure.TIM_Prescaler = TIM8_PWM_PSC_Value; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	//初始化TIM8 Channel2 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM8 OC1

	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);  				//使能TIM8在CCR1上的预装载寄存器
	
	TIM_SetCompare1(TIM8, M_FAN_speed_stop);										//占空比设置为0%

	TIM_Cmd(TIM8, ENABLE);  //使能TIM8
}

//机器人风机控制函数：
//m_fan_run->0停止,1运行;
//m_fan_speed->风机运行速度
void MOTOR_FAN(u8 m_fan_run,u16 m_fan_speed)								
{
	if(m_fan_run){
		TIM_SetCompare1(TIM8, m_fan_speed);
	}
	else{
		TIM_SetCompare1(TIM8, M_FAN_speed_stop);
	}
}

//机器人风机软启动函数
//m_fan_run->0停止,1运行;
//m_fan_speed_max->风机最终需达到的速度
//m_fan_step->m_fan_speed_max右移数,1-->2步,2-->4步,3-->8步,4-->16步
//timer_flag->定时延时标识
void MOTOR_FAN_SOFT_START(u8 m_fan_run, u16 m_fan_speed_max, u16 m_fan_step, u8 timer_flag)
{
	u16 m_fan_speed_temp;
	if(m_fan_run){															//软启动开始
		m_fan_speed_temp = m_fan_speed_max >> m_fan_step ;
		if(timer_flag){
			if(m_fan_step){
				TIM_SetCompare1(TIM8, m_fan_speed_temp);
				m_fan_speed_temp += m_fan_speed_temp;
				timer_flag = 0;
			}else{
				m_fan_step--;
			}
		}else{
				;
		}
	}
	else{
		TIM_SetCompare1(TIM8, M_FAN_speed_stop);
	}
}
 
