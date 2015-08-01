#include "charger.h"

//�������PWM���
void CHARGER_Start(void)																																					
{
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
//	TIM1->CCER |= (uint16_t)((uint16_t)TIM_CCER_CC1E);
}

//�رճ��PWM���
void CHARGER_Stop(void)																																						
{
	TIM_CtrlPWMOutputs(TIM1, DISABLE);
//	TIM1->CCER &= (uint16_t)~((uint16_t)TIM_CCER_CC1E);
}

//���ͨ�Ͽ��ƶ˿�PWM����
void CHARGER_PWM_Init(u16 arr, u16 psc)																														
{
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  				TIM_OCInitStructure;
	GPIO_InitTypeDef  				GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_TIM1|RCC_APB2Periph_AFIO, ENABLE);		//ʹ��PA�˿�ʱ�Ӻ�TIM1ʱ�Ӻ�AFIO���ù���ʱ��

	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_8;																											//CHARGER_CUTOFF-->PA.8 �˿�����
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;																								//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);

	 //��ʼ��TIM1
 TIM_TimeBaseStructure.TIM_Period = arr; 																													//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
 TIM_TimeBaseStructure.TIM_Prescaler = psc; 																											//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 																					//����ʱ�ӷָ�:TDTS = Tck_tim
 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  																		//TIM���ϼ���ģʽ
 TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); 																									//����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

 //��ʼ��TIM8 Channel2,Channel3 PWMģʽ
 TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 																								//ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ1
 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 																		//�Ƚ����ʹ��
 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 																				//�������:TIM����Ƚϼ��Ը�
 TIM_OC1Init(TIM1, &TIM_OCInitStructure);  											 																	//����ָ���Ĳ�����ʼ������TIM1 OC1

 TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  																							//ʹ��TIM1��CCR1�ϵ�Ԥװ�ؼĴ���

 TIM_Cmd(TIM1, ENABLE);																																						//ʹ��TIM1
}

//���ӿڳ�ʼ��
//ʵ�����PWMΪ30khz��60%ռ�ձ�,����ص�ѹ����1/4��ѹʱ,����13%~60%,����1.5%ռ�ձȵķ�ʽ���
void CHARGER_Init(void)																												
{
	CHARGER_PWM_Init(33,71);																											//��ʼ��TIM1_CH1->PA8,PWMƵ��30Khz
	CHARGER_Stop();																															//���ܳ��PWM���
	TIM_SetCompare1(TIM1, 15);																									//ռ�ձ�����Ϊ45.5%,��﮵��14.4V��ѹ�����������ĳ���ѹ,����ѹ=﮵�ض��ѹ*1.15=14.4*1.15=16.56V,16.56/24=0.69																			
}

//��ص�������1/4�������ʱ�ĳ����ƺ���
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

//��ص�������1/4�������ʱ,�����ƶ����ռ�ձ�����Ϊ45.5%
void CHARGER_Start_more_than_1d4(void)
{
	TIM_SetCompare1(TIM1, 15);
	CHARGER_Start();
}

//��ص�������3/4�������ʱ�������ƶ����ռ�ձ�����Ϊ15%,������
void CHARGER_Startt_more_than_3d4(void)
{
	TIM_SetCompare1(TIM1, 5);
	CHARGER_Start();
}
