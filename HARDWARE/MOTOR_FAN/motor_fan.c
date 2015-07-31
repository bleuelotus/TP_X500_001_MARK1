#include "motor_fan.h"

void MOTOR_FAN_Init(void)
{
 GPIO_InitTypeDef 				GPIO_InitStructure;
 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 TIM_OCInitTypeDef  			TIM_OCInitStructure;

 RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);			//ʹ�ܶ�ʱ��8ʱ��
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 	//ʹ��PC�˿�ʱ��

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				 				//����������ƶ˿�-->PC.6 �˿�����,TIM8_CH1
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 			//�����������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 					//�����趨������ʼ��GPIOC.6

	//��ʼ��TIM8
	TIM_TimeBaseStructure.TIM_Period = TIM8_PWM_ARR_Value; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ,1Khz
	TIM_TimeBaseStructure.TIM_Prescaler = TIM8_PWM_PSC_Value; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	//��ʼ��TIM8 Channel2 PWMģʽ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM8 OC1

	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);  				//ʹ��TIM8��CCR1�ϵ�Ԥװ�ؼĴ���
	
	TIM_SetCompare1(TIM8, M_FAN_speed_stop);										//ռ�ձ�����Ϊ0%

	TIM_Cmd(TIM8, ENABLE);  //ʹ��TIM8
}

//�����˷�����ƺ�����
//m_fan_run->0ֹͣ,1����;
//m_fan_speed->��������ٶ�
void MOTOR_FAN(u8 m_fan_run,u16 m_fan_speed)								
{
	if(m_fan_run){
		TIM_SetCompare1(TIM8, m_fan_speed);
	}
	else{
		TIM_SetCompare1(TIM8, M_FAN_speed_stop);
	}
}

//�����˷������������
//m_fan_run->0ֹͣ,1����;
//m_fan_speed_max->���������ﵽ���ٶ�
//m_fan_step->m_fan_speed_max������,1-->2��,2-->4��,3-->8��,4-->16��
//timer_flag->��ʱ��ʱ��ʶ
void MOTOR_FAN_SOFT_START(u8 m_fan_run, u16 m_fan_speed_max, u16 m_fan_step, u8 timer_flag)
{
	u16 m_fan_speed_temp;
	if(m_fan_run){															//��������ʼ
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
 
