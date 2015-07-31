#include "sensor.h"

//���ADCֵ
//ch:ͨ��ֵ
u16 Get_Adc(u8 ch)
{
//����ָ��ADC�Ĺ�����ͨ��,һ������,����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
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

//���Ϸ����·PWM��������ʼ��,PC.9->TIM8_CH4
void SENSOR_PWM_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  				TIM_OCInitStructure;
 	GPIO_InitTypeDef 					GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);			//ʹ�ܶ�ʱ��8ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);	 	//ʹ��PC�˿�ʱ�Ӻ�AFIO���ù���ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 					//����PWM����˿�-->PC.9 �˿�����,TIM8_CH4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 			//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 					//�����趨������ʼ��GPIOC.9

	//��ʼ��TIM8
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	//��ʼ��TIM8 Channel4 PWMģʽ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	TIM_OC4Init(TIM8, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM8 OC4

	TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);  //ʹ��TIM8��CCR4�ϵ�Ԥװ�ؼĴ���

	TIM_Cmd(TIM8, ENABLE);  //ʹ��TIM8
}

//���������ADC�˿ڳ�ʼ��
void SENSOR_ADC_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE );	  //ʹ��ADC1ͨ��ʱ��

	RCC_ADCCLKConfig(RCC_PCLK2_Div8);																																			//����ADC��Ƶ����8 72M/8=9M,ADC���ʱ�䲻�ܳ���14M

	//PA3,PA4,PA5,PA6,PA7 ��Ϊģ��ͨ����������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PC0,PC4,PC5 ��Ϊģ��ͨ����������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1

	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼

	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����

	ADC_StartCalibration(ADC1);	 //����ADУ׼

	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����

//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������
}

//����������ⲿ�ж϶˿ڳ�ʼ��
void SENSOR_EXTI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);														//ʹ�ܸ��ù���ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);													//ʹ��GPIOAʱ��

	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_1;																	//GPIOA.1->˫��̧��״̬��
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;															//��������
	GPIO_Init(GPIOA,&GPIO_InitStructure);																					//GPIOA.1��ʼ��

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);

	EXTI_InitStructure.EXTI_Line 		= EXTI_Line1;																	//GPIOA.1
  EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;												//����Ϊ�ж�ģʽ
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;												//�ж��½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;															//ʹ��GPIOA.1���ڵ��ⲿ�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;									//��ռ���ȼ�1,
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;													//�����ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;																//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);
}

//������ʼ������
void SENSOR_Init(void) //SENSOR���IO��ʼ��
{
 	GPIO_InitTypeDef 					GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE,ENABLE);//ʹ��PROTA|PORTB|PORTC|PORTD|PORTEʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;//PA2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA2

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14|GPIO_Pin_15;//PB14~15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB14~15

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10|GPIO_Pin_12;//PC10~12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC10~12

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PD3~7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIOD3~7

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//PE12~15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE12~15

	SENSOR_ADC_Init();																															//���������ADC�˿ڳ�ʼ��

	SENSOR_PWM_Init(TIM8_PWM_ARR_Value,TIM8_PWM_PSC_Value);													//���Ϸ����·����10Khz��PWM����
	TIM_SetCompare4(TIM8,SENS_LED_Value_const);																			//���Ϸ����·PWMռ�ձ�
}

//SENSOR������(�������������λ�������ͷ,�������ͷ��ʼ������remote.c�ļ���)
//���ض�ӦSENSORֵ
u16 SENSOR_Scan(u8 sensor_var)
{
//	u8 ADC_average_times = 10;
	u16 sensor_value;
	switch(sensor_var){
		case(SENS_F_L):																																//ǰ������
			sensor_value = Get_Adc(SENS_F_L_ADC_CH);
		break;
		case(SENS_F_LM):																															//ǰ��������
			sensor_value = Get_Adc(SENS_F_LM_ADC_CH);
		break;
		case(SENS_F_M):																																//ǰ������
			sensor_value = Get_Adc(SENS_F_M_ADC_CH);
		break;
		case(SENS_F_RM):																															//ǰ��������
			sensor_value = Get_Adc(SENS_F_RM_ADC_CH);
		break;
		case(SENS_F_R):																																//ǰ������
			sensor_value = Get_Adc(SENS_F_R_ADC_CH);
		break;
		case(SENS_FI_L):																															//ǰ������ײ��
			sensor_value = 0x0001&(SENS_FI_L_Value);
		break;
		case(SENS_FI_R):																															//ǰ������ײ��
			sensor_value = 0x0001&(SENS_FI_R_Value);
		break;
		case(SENS_B_L):																																//̽�׼����
			sensor_value = 0x0001&(SENS_B_L_Value);
		break;
		case(SENS_B_LM):																															//̽�׼������
			sensor_value = 0x0001&(SENS_B_LM_Value);
		break;
		case(SENS_B_RM):																															//̽�׼������
			sensor_value = 0x0001&(SENS_B_RM_Value);
		break;
		case(SENS_B_R):																																//̽�׼����
			sensor_value = 0x0001&(SENS_B_R_Value);
		break;
		case(SENS_WHEEL_UP):																													//˫��̧��
			sensor_value = 0x0001&(SENS_WHEEL_UP_Value);
		break;
		case(SENS_NO_GARBAGE_BOX):																										//������δ�ż��
			sensor_value = 0x0001&(SENS_NO_GARBAGE_BOX_Value);
		break;
		case(SENS_DUSH_FULL):																													//�ҳ�������
			sensor_value = 0x0001&(SENS_DUSH_FULL_Value);
		break;
//		case(SENS_CHARGE_BOX_CON):																										//��������Ӽ��
//			sensor_value = 0x0001&(SENS_CHARGE_BOX_CON_Value);
//		break;
//		case(SENS_CB_LOC_L):																													//�������λ������
//			sensor_value = 0x0001&(SENS_CB_LOC_L_Value);
//		break;
//		case(SENS_CB_LOC_LM):																													//�������λ����
//			sensor_value = 0x0001&(SENS_CB_LOC_LM_Value);
//		break;
//		case(SENS_CB_LOC_RM):																													//�������λ����
//			sensor_value = 0x0001&(SENS_CB_LOC_RM_Value);
//		break;
//		case(SENS_CB_LOC_R):																													//�������λ������
//			sensor_value = 0x0001&(SENS_CB_LOC_R_Value);
//		break;
		case(SENS_FAN_OVLOAD):																												//����������ؼ��
			sensor_value = 0x0001&(SENS_FAN_OVLOAD_Value);
		break;
		case(SENS_M_SIDE_L_OVLOAD):																										//���ˢ������ؼ��
			sensor_value = 0x0001&(SENS_M_SIDE_L_OVLOAD_Value);
		break;
		case(SENS_M_SIDE_R_OVLOAD):																										//�ұ�ˢ������ؼ��
			sensor_value = 0x0001&(SENS_M_SIDE_R_OVLOAD_Value);
		break;
		case(SENS_M_MOVE_L_OVLOAD):																										//�����ߵ�����ؼ��
			sensor_value = 0x0001&(SENS_M_MOVE_L_OVLOAD_Value);
		break;
		case(SENS_M_MOVE_R_OVLOAD):																										//�����ߵ�����ؼ��
			sensor_value = 0x0001&(SENS_M_MOVE_R_OVLOAD_Value);
		break;
		case(SENS_Battery):																														//��ص������
			if(g_charge_start_flag)
				TIM_CtrlPWMOutputs(TIM1, DISABLE);																						//����翪ʼ������ʱ�ȹر�PWM
			sensor_value = Get_Adc(SENS_Battery_ADC_CH);
			if(g_charge_start_flag)
				TIM_CtrlPWMOutputs(TIM1, ENABLE);																							//�������PWM
		break;
		case(SENS_24V):																																//���ʱ��Դ�������
			sensor_value = Get_Adc(SENS_Charging_Battery_ADC_CH);
		break;
		case(SENS_Battery_temp):																											//����¶ȼ��
			sensor_value = Get_Adc(SENS_Battery_temp_ADC_CH);
		break;
		default:
			sensor_value = 0;
		break;
	}
	return(sensor_value);
}


////SENSOR������(�����������λ�������ͷ,ֱ�Ӷ�IO��ƽ��������)
////���ض�ӦSENSORֵ
//u16 SENSOR_Scan(u8 sensor_var)
//{
////	u8 ADC_average_times = 10;
//	u16 sensor_value;
//	switch(sensor_var){
//		case(SENS_F_L):																																//ǰ������
//			sensor_value = Get_Adc(SENS_F_L_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_L_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_L_Value);
//		break;
//		case(SENS_F_LM):																															//ǰ��������
//			sensor_value = Get_Adc(SENS_F_LM_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_LM_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_LM_Value);
//		break;
//		case(SENS_F_M):																																//ǰ������
//			sensor_value = Get_Adc(SENS_F_M_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_M_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_M_Value);
//		break;
//		case(SENS_F_RM):																															//ǰ��������
//			sensor_value = Get_Adc(SENS_F_RM_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_RM_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_RM_Value);
//		break;
//		case(SENS_F_R):																																//ǰ������
//			sensor_value = Get_Adc(SENS_F_R_ADC_CH);
////			sensor_value = Get_Adc_Average(SENS_F_R_ADC_CH, ADC_average_times);
////			sensor_value = 0x0001&(SENS_F_R_Value);
//		break;
//		case(SENS_FI_L):																															//ǰ������ײ��
//			sensor_value = 0x0001&(SENS_FI_L_Value);
//		break;
//		case(SENS_FI_R):																															//ǰ������ײ��
//			sensor_value = 0x0001&(SENS_FI_R_Value);
//		break;
//		case(SENS_B_L):																																//̽�׼����
//			sensor_value = 0x0001&(SENS_B_L_Value);
//		break;
//		case(SENS_B_LM):																															//̽�׼������
//			sensor_value = 0x0001&(SENS_B_LM_Value);
//		break;
//		case(SENS_B_RM):																															//̽�׼������
//			sensor_value = 0x0001&(SENS_B_RM_Value);
//		break;
//		case(SENS_B_R):																																//̽�׼����
//			sensor_value = 0x0001&(SENS_B_R_Value);
//		break;
//		case(SENS_WHEEL_UP):																													//˫��̧��
//			sensor_value = 0x0001&(SENS_WHEEL_UP_Value);
//		break;
//		case(SENS_NO_GARBAGE_BOX):																										//������δ�ż��
//			sensor_value = 0x0001&(SENS_NO_GARBAGE_BOX_Value);
//		break;
//		case(SENS_DUSH_FULL):																													//�ҳ�������
//			sensor_value = 0x0001&(SENS_DUSH_FULL_Value);
//		break;
//		case(SENS_CHARGE_BOX_CON):																										//��������Ӽ��
//			sensor_value = 0x0001&(SENS_CHARGE_BOX_CON_Value);
//		break;
//		case(SENS_CB_LOC_L):																													//�������λ������
//			sensor_value = 0x0001&(SENS_CB_LOC_L_Value);
//		break;
//		case(SENS_CB_LOC_LM):																													//�������λ����
//			sensor_value = 0x0001&(SENS_CB_LOC_LM_Value);
//		break;
//		case(SENS_CB_LOC_RM):																													//�������λ����
//			sensor_value = 0x0001&(SENS_CB_LOC_RM_Value);
//		break;
//		case(SENS_CB_LOC_R):																													//�������λ������
//			sensor_value = 0x0001&(SENS_CB_LOC_R_Value);
//		break;
//		case(SENS_FAN_OVLOAD):																												//����������ؼ��
//			sensor_value = 0x0001&(SENS_FAN_OVLOAD_Value);
//		break;
//		case(SENS_M_SIDE_L_OVLOAD):																										//���ˢ������ؼ��
//			sensor_value = 0x0001&(SENS_M_SIDE_L_OVLOAD_Value);
//		break;
//		case(SENS_M_SIDE_R_OVLOAD):																										//�ұ�ˢ������ؼ��
//			sensor_value = 0x0001&(SENS_M_SIDE_R_OVLOAD_Value);
//		break;
//		case(SENS_M_MOVE_L_OVLOAD):																										//�����ߵ�����ؼ��
//			sensor_value = 0x0001&(SENS_M_MOVE_L_OVLOAD_Value);
//		break;
//		case(SENS_M_MOVE_R_OVLOAD):																										//�����ߵ�����ؼ��
//			sensor_value = 0x0001&(SENS_M_MOVE_R_OVLOAD_Value);
//		break;
//		case(SENS_Battery):																														//��ص������
//			sensor_value = Get_Adc(SENS_Battery_ADC_CH);
//		break;
//		case(SENS_24V):																																//���ʱ��Դ�������
//			sensor_value = Get_Adc(SENS_Charging_Battery_ADC_CH);
//		break;
//		case(SENS_Battery_temp):																											//����¶ȼ��
//			sensor_value = Get_Adc(SENS_Battery_temp_ADC_CH);
//		break;
//		default:
//			sensor_value = 0;
//		break;
//	}
//	return(sensor_value);
//}
