#include "motor_move.h"

//���ߵ���ٶȿ���PWM��������ʼ�����ú���,PWM�����ʱ�������,�����ʱ���ֹͣ
void MOTOR_MOVE_PWM_Init(u16 arr, u16 psc)
{
 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 TIM_OCInitTypeDef  			TIM_OCInitStructure;
 GPIO_InitTypeDef  				GPIO_InitStructure;

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);														//ʹ�ܶ�ʱ��8ʱ��
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);		//ʹ��PC�˿�ʱ�Ӻ�AFIO���ù���ʱ��

 GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_7 | GPIO_Pin_8;											//�����ٶȿ��ƶ˿�-->PC.7 �˿�����,TIM8_CH2 && �����ߵ�����ƶ˿�-->PC.8 �˿�����,TIM8_CH3
 GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_OD; 		 													//���ÿ�©���
 GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;		 													//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 																//�����趨������ʼ��GPIOC.7,GPIOC.8

 //��ʼ��TIM8
 TIM_TimeBaseStructure.TIM_Period 					= arr; 															//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
 TIM_TimeBaseStructure.TIM_Prescaler 				= psc; 			 												//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
 TIM_TimeBaseStructure.TIM_ClockDivision 		= TIM_CKD_DIV1; 										//����ʱ�ӷָ�:TDTS = Tck_tim
 TIM_TimeBaseStructure.TIM_CounterMode 			= TIM_CounterMode_Up;  							//TIM���ϼ���ģʽ
 TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); 																//����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

 //��ʼ��TIM8 Channel2,Channel3 PWMģʽ
 TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 															//ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ1,TIM_CNT<TIM_CCRʱ,PWM�����,TIM_CCR<TIM_CNT<TIM_ARRʱ,PWM�����
 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 									//�Ƚ����ʹ��
 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 											//�������:TIM����Ƚϼ��Ը�
 TIM_OC2Init(TIM8, &TIM_OCInitStructure);  											 								//����ָ���Ĳ�����ʼ������TIM8 OC2
 TIM_OC3Init(TIM8, &TIM_OCInitStructure);																				//����ָ���Ĳ�����ʼ������TIM8 OC3

 TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);  														//ʹ��TIM8��CCR2�ϵ�Ԥװ�ؼĴ���
 TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);  														//ʹ��TIM8��CCR3�ϵ�Ԥװ�ؼĴ���

 TIM_Cmd(TIM8, ENABLE);  																												//ʹ��TIM8
}

//���ߵ���ٶȷ�������
void MOTOR_SPEED_Monitor_Init(void)
{
//TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	GPIO_InitTypeDef  				GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);							//ʹ�ܶ�ʱ��3�붨ʱ��4ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);	 						//ʹ��PD,PE�˿�ʱ�Ӻ�AFIO���ù���ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;																								//�����ٶȷ����˿�-->PE.0 �˿�����,TIM4_ETR
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;																						//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;																				//IO���ٶ�Ϊ10Mhz
	GPIO_Init(GPIOE, &GPIO_InitStructure);																									//�����趨������ʼ��GPIOE.0,GPIOC.8

	TIM_ETRClockMode2Config(TIM4, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_Inverted, 0x00);		//TIM4�ⲿʱ��ģʽ2����

//	TIM_PrescalerConfig(TIM4, 71, TIM_PSCReloadMode_Update);																//����TIM4��Ƶ��

	TIM_Cmd(TIM4, ENABLE);  																																//ʹ��TIM4

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;																								//�����ߵ�����ƶ˿�-->PD.2 �˿�����,TIM3_ETR
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;																						//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;																				//IO���ٶ�Ϊ10Mhz
	GPIO_Init(GPIOD, &GPIO_InitStructure);																									//�����趨������ʼ��GPIOD.2

	TIM_ETRClockMode2Config(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_Inverted, 0x00);		//TIM3�ⲿ����ģʽ����

//	TIM_PrescalerConfig(TIM3, 71, TIM_PSCReloadMode_Update);																//����TIM3��Ƶ��

	TIM_Cmd(TIM3, ENABLE);  																																//ʹ��TIM3
}

//���ߵ����ض˿ڳ�ʼ������
void MOTOR_MOVE_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 												//ʹ��PE�˿�ʱ��

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 															//��������ת���ƶ˿�-->PE.1 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//��©���
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//�����趨������ʼ��GPIOE.1
 GPIO_ResetBits(GPIOE,GPIO_Pin_1);						 																	//PE.1 �����->��ת,�����->��ת

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 															//������ͣ���ƶ˿�-->PE.2 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//��©���
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//�����趨������ʼ��GPIOE.2
 GPIO_SetBits(GPIOE,GPIO_Pin_2);						 																		//PE.2 �����->ֹͣ,�����->����

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 															//�ҵ������ת���ƶ˿�-->PE.9 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//��©���
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//�����趨������ʼ��GPIOE.9
 GPIO_ResetBits(GPIOE,GPIO_Pin_9);						 																	//PE.9 �����->��ת,�����->��ת

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				 															//�ҵ����ͣ���ƶ˿�-->PE.10 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//��©���
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//�����趨������ʼ��GPIOE.10
 GPIO_SetBits(GPIOE,GPIO_Pin_10);						 																		//PE.10 �����->ֹͣ,�����->����

 MOTOR_MOVE_PWM_Init(TIM8_PWM_ARR_Value,TIM8_PWM_PSC_Value);										//��ʼ�����ߵ���˿�_�ٶ�_���10khzPWM

 MOTOR_SPEED_Monitor_Init();																										//��ʼ�����ߵ���ٶȷ����˿�
}

//����ٶ��趨����,m_move_motor_select->����Ϊ1,�ҵ��Ϊ0;m_speed->���100
void MOTOR_MOVE_SPEED_Set(u8 m_move_motor_select,u8 m_speed)
{
	if(m_move_motor_select)
		TIM_SetCompare2(TIM8, m_speed);
	else
		TIM_SetCompare3(TIM8, m_speed);
}

//������ǰ��_0
void MOTOR_MOVE_FORWARD(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//�����˺���_1
void MOTOR_MOVE_BACKWARD(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;
}

//������˳ʱ����ת_2
void MOTOR_MOVE_CLOCKWIZE(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;
}

//��������ʱ����ת_3
void MOTOR_MOVE_ANTICLOCKWIZE(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//��������ǰ��_4
void MOTOR_MOVE_TURNLEFT(u8 m_move_speed)
{
	u8 m_move_speed_l_tmp;

	m_move_speed_l_tmp = m_move_speed - 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed_l_tmp);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed);
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//��������ǰ��_5
void MOTOR_MOVE_TURNRIGHT(u8 m_move_speed)
{
	u8 m_move_speed_r_tmp;

	m_move_speed_r_tmp = m_move_speed - 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed_r_tmp);
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//�����������_6
void MOTOR_MOVE_BACKLEFT(u8 m_move_speed){
	u8 m_move_speed_r_tmp;

	m_move_speed_r_tmp = m_move_speed + 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed_r_tmp);
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;	
}

//�������Һ���_7
void MOTOR_MOVE_BACKRIGHT(u8 m_move_speed){
	u8 m_move_speed_l_tmp;

	m_move_speed_l_tmp = m_move_speed + 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed_l_tmp);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed);
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;
}

//������ֹͣ_8
void MOTOR_MOVE_STOP(void)
{
	TIM_SetCompare2(TIM8, 0);
	TIM_SetCompare3(TIM8, 0);
	M_MOVE_STOP_L 	= 	M_MOVE_Stop;
	M_MOVE_STOP_R 	= 	M_MOVE_Stop;
}

//����������_9
void MOTOR_MOVE_RUN(u8 m_move_speed)
{
	TIM_SetCompare2(TIM8, m_move_speed);
	TIM_SetCompare3(TIM8, m_move_speed);
	M_MOVE_STOP_L 	= 	M_MOVE_Run;
	M_MOVE_STOP_R 	= 	M_MOVE_Run;	
}

//���������߿��ƺ�����
//run_flag->0����,1ֹͣ;
//direc_flag->0ǰ��,1����,2˳ʱ����ת,3��ʱ����ת, 4��ǰ��, 5��ǰ��, 6����� , 7�Һ���;
//speed->�������ߵ���ٶ�
void MOTOR_MOVE(u8 m_move_direc, u16 m_move_speed)
{
	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed);		//���������ߵ��ת��
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed);		//���������ߵ��ת��

	switch(m_move_direc){
		case(1):
			MOTOR_MOVE_BACKWARD();
			break;
		case(2):
			MOTOR_MOVE_CLOCKWIZE();
			break;
		case(3):
			MOTOR_MOVE_ANTICLOCKWIZE();
			break;
		case(4):
			MOTOR_MOVE_TURNLEFT(m_move_speed);
			break;
		case(5):
			MOTOR_MOVE_TURNRIGHT(m_move_speed);
			break;
		case(6):
			MOTOR_MOVE_BACKLEFT(m_move_speed);
			break;
		case(7):
			MOTOR_MOVE_BACKRIGHT(m_move_speed);
			break;
		default:
			MOTOR_MOVE_FORWARD();
			break;
	}
}

u16 MOTOR_MOVE_DIST_L_READ(void)												//�����ߵ����̼���ֵ��ȡ����
{
	u16 motor_move_dist_val = 0;
	motor_move_dist_val = TIM_GetCounter(TIM4);
	return(motor_move_dist_val);
}

u16 MOTOR_MOVE_DIST_R_READ(void)												//�����ߵ����̼���ֵ��ȡ����
{
	u16 motor_move_dist_val = 0;
	motor_move_dist_val = TIM_GetCounter(TIM3);
	return(motor_move_dist_val);
}

//���ߵ�����϶�������
//����->˳ʱ����תm_move_clkwiz_angle
void MOTOR_MOVE_AVOID(u16 m_move_backward_angle, u16 m_move_turn_angle, u16 m_move_direc, u16 m_move_speed)
{
	MOTOR_TURN_Angle(m_move_backward_angle, M_MOVE_Backward, m_move_speed);
	MOTOR_TURN_Angle(m_move_turn_angle, m_move_direc, m_move_speed);
}

//���������ߵ������������,ʹ�ö�ʱ��
//m_motor_run->0ֹͣ,1����;
//m_motor_speed_max->���������ﵽ���ٶ�
//m_motor_time->��������ʱ��,��С1s
//void MOTOR_MOVE_SOFT_START(u8 m_motor_run, u16 m_motor_speed_max, u16 m_motor_time)
//{
//	u16 m_motor_speed_temp;
//	if(m_motor_run == 0){																													//��������ʼ
//		MOTOR_MOVE_FORWARD();
//		m_motor_speed_temp = m_motor_speed_max/m_motor_time ;
//		for(;m_motor_time > 0;m_motor_time--){
//			if(timer_1s_flag){
//				TIM_SetCompare2(TIM8, m_motor_speed_temp);
//				m_motor_speed_temp += m_motor_speed_temp;
//				timer_1s_flag = 0;
//			}else{
//				;
//			}
//		}
//	}
//	else{
//		TIM_SetCompare2(TIM8,0);																											//�����ߵ��ֹͣ
//		TIM_SetCompare3(TIM8,0);																											//�����ߵ��ֹͣ
//	}
//}

void MOTOR_MOVE_SOFT_START_SIMPLE(void)
{
	u16 m_motor_speed_temp;

	MOTOR_MOVE_FORWARD();
	for(m_motor_speed_temp = 0; m_motor_speed_temp < 99; m_motor_speed_temp++){
		TIM_SetCompare2(TIM8, m_motor_speed_temp);
		TIM_SetCompare3(TIM8, m_motor_speed_temp);
		delay_ms(50);
	}
}

//�������ߵ��ת��m_move_angle�ȵĺ���
	//m_move_length_r->�����ְ뾶,ȫ�ֱ���
	//m_move_length_l->���������ĵ����������ĵľ���,ȫ�ֱ���
	//m_move_encoder_num->������תһȦ��Ӧ���̼���ֵ,ȫ�ֱ���
	//m_move_angle->������ת��Ƕ�
	//m_move_direc->������ת����
void MOTOR_TURN_Angle(u16 m_move_angle, u16 m_move_direc, u16 m_move_speed)
{
	u32 waiting_time = 0;
	u16 encoder_target_num = 0;
	u32 calc_tmp_num = 0;																													//���ڼ����м�������ʱ����
	u16 encoder_tim3_cnt = 0;
	u16 encoder_tim4_cnt = 0;

	TIM_SetCompare2(TIM8,m_move_speed);																						//���������ߵ��ת��
	TIM_SetCompare3(TIM8,m_move_speed);																						//���������ߵ��ת��

	calc_tmp_num = m_move_angle*m_move_length_l*m_move_encoder_num;
	encoder_target_num = (u16)(calc_tmp_num /(360*m_move_length_r));
	
	TIM_SetCounter(TIM3, 0);																											//�������̼���ֵ
	TIM_SetCounter(TIM4, 0);																											//�������̼���ֵ

	switch(m_move_direc){
		case(1):																																			//����,���ݸ���angle��ͬ���˲�ͬ����,���˾���=(angle*pi*length_l/180)
			MOTOR_MOVE_BACKWARD();
			break;
		case(2):																																			//˳ʱ��
			MOTOR_MOVE_CLOCKWIZE();
			break;
		case(3):																																			//��ʱ��
			MOTOR_MOVE_ANTICLOCKWIZE();
			break;
		case(4):																																			//��ǰ��
			MOTOR_MOVE_TURNLEFT(m_move_speed);
			break;
		case(5):																																			//��ǰ��
			MOTOR_MOVE_TURNRIGHT(m_move_speed);
			break;
		case(6):																																			//�����
			MOTOR_MOVE_BACKLEFT(m_move_speed);
			break;
		case(7):																																			//�Һ���
			MOTOR_MOVE_BACKRIGHT(m_move_speed);
			break;
		default:																																			//˳ʱ��
			MOTOR_MOVE_CLOCKWIZE();
			break;
	}

	for(waiting_time = 0x300000;waiting_time>0;waiting_time--){											//�ȴ����̼���ֵ�ﵽתangle�Ƕ���Ҫ�ļ���ֵ,����ʱ���Զ��˳�
		if((encoder_tim3_cnt < encoder_target_num)&&(encoder_tim4_cnt < encoder_target_num)){
			encoder_tim3_cnt = TIM_GetCounter(TIM3);
			encoder_tim4_cnt = TIM_GetCounter(TIM4);
		}else
			break;
	}
	printf("target=%d,wait=%d,tim3cnt=%d,tim4cnt=%d\r\n",encoder_target_num,waiting_time,encoder_tim3_cnt,encoder_tim4_cnt);
}
