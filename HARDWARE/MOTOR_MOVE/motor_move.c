#include "motor_move.h"

//行走电机速度控制PWM发生器初始化配置函数,PWM输出低时电机运行,输出高时电机停止
void MOTOR_MOVE_PWM_Init(u16 arr, u16 psc)
{
 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 TIM_OCInitTypeDef  			TIM_OCInitStructure;
 GPIO_InitTypeDef  				GPIO_InitStructure;

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);														//使能定时器8时钟
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);		//使能PC端口时钟和AFIO复用功能时钟

 GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_7 | GPIO_Pin_8;											//左电机速度控制端口-->PC.7 端口配置,TIM8_CH2 && 右行走电机控制端口-->PC.8 端口配置,TIM8_CH3
 GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_OD; 		 													//复用开漏输出
 GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;		 													//IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 																//根据设定参数初始化GPIOC.7,GPIOC.8

 //初始化TIM8
 TIM_TimeBaseStructure.TIM_Period 					= arr; 															//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
 TIM_TimeBaseStructure.TIM_Prescaler 				= psc; 			 												//设置用来作为TIMx时钟频率除数的预分频值
 TIM_TimeBaseStructure.TIM_ClockDivision 		= TIM_CKD_DIV1; 										//设置时钟分割:TDTS = Tck_tim
 TIM_TimeBaseStructure.TIM_CounterMode 			= TIM_CounterMode_Up;  							//TIM向上计数模式
 TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); 																//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

 //初始化TIM8 Channel2,Channel3 PWM模式
 TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 															//选择定时器模式:TIM脉冲宽度调制模式1,TIM_CNT<TIM_CCR时,PWM输出低,TIM_CCR<TIM_CNT<TIM_ARR时,PWM输出高
 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 									//比较输出使能
 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 											//输出极性:TIM输出比较极性高
 TIM_OC2Init(TIM8, &TIM_OCInitStructure);  											 								//根据指定的参数初始化外设TIM8 OC2
 TIM_OC3Init(TIM8, &TIM_OCInitStructure);																				//根据指定的参数初始化外设TIM8 OC3

 TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);  														//使能TIM8在CCR2上的预装载寄存器
 TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);  														//使能TIM8在CCR3上的预装载寄存器

 TIM_Cmd(TIM8, ENABLE);  																												//使能TIM8
}

//行走电机速度反馈函数
void MOTOR_SPEED_Monitor_Init(void)
{
//TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	GPIO_InitTypeDef  				GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);							//使能定时器3与定时器4时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);	 						//使能PD,PE端口时钟和AFIO复用功能时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;																								//左电机速度反馈端口-->PE.0 端口配置,TIM4_ETR
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;																						//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;																				//IO口速度为10Mhz
	GPIO_Init(GPIOE, &GPIO_InitStructure);																									//根据设定参数初始化GPIOE.0,GPIOC.8

	TIM_ETRClockMode2Config(TIM4, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_Inverted, 0x00);		//TIM4外部时钟模式2配置

//	TIM_PrescalerConfig(TIM4, 71, TIM_PSCReloadMode_Update);																//设置TIM4分频比

	TIM_Cmd(TIM4, ENABLE);  																																//使能TIM4

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;																								//右行走电机控制端口-->PD.2 端口配置,TIM3_ETR
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;																						//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;																				//IO口速度为10Mhz
	GPIO_Init(GPIOD, &GPIO_InitStructure);																									//根据设定参数初始化GPIOD.2

	TIM_ETRClockMode2Config(TIM3, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_Inverted, 0x00);		//TIM3外部触发模式配置

//	TIM_PrescalerConfig(TIM3, 71, TIM_PSCReloadMode_Update);																//设置TIM3分频比

	TIM_Cmd(TIM3, ENABLE);  																																//使能TIM3
}

//行走电机相关端口初始化函数
void MOTOR_MOVE_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 												//使能PE端口时钟

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 															//左电机正反转控制端口-->PE.1 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//开漏输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//根据设定参数初始化GPIOE.1
 GPIO_ResetBits(GPIOE,GPIO_Pin_1);						 																	//PE.1 输出高->反转,输出低->正转

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				 															//左电机启停控制端口-->PE.2 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//开漏输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//根据设定参数初始化GPIOE.2
 GPIO_SetBits(GPIOE,GPIO_Pin_2);						 																		//PE.2 输出高->停止,输出低->运行

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 															//右电机正反转控制端口-->PE.9 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//开漏输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//根据设定参数初始化GPIOE.9
 GPIO_ResetBits(GPIOE,GPIO_Pin_9);						 																	//PE.9 输出高->反转,输出低->正转

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				 															//右电机启停控制端口-->PE.10 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 													//开漏输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 													//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 																//根据设定参数初始化GPIOE.10
 GPIO_SetBits(GPIOE,GPIO_Pin_10);						 																		//PE.10 输出高->停止,输出低->运行

 MOTOR_MOVE_PWM_Init(TIM8_PWM_ARR_Value,TIM8_PWM_PSC_Value);										//初始化行走电机端口_速度_输出10khzPWM

 MOTOR_SPEED_Monitor_Init();																										//初始化行走电机速度反馈端口
}

//电机速度设定函数,m_move_motor_select->左电机为1,右电机为0;m_speed->最大100
void MOTOR_MOVE_SPEED_Set(u8 m_move_motor_select,u8 m_speed)
{
	if(m_move_motor_select)
		TIM_SetCompare2(TIM8, m_speed);
	else
		TIM_SetCompare3(TIM8, m_speed);
}

//机器人前进_0
void MOTOR_MOVE_FORWARD(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//机器人后退_1
void MOTOR_MOVE_BACKWARD(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;
}

//机器人顺时针旋转_2
void MOTOR_MOVE_CLOCKWIZE(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;
}

//机器人逆时针旋转_3
void MOTOR_MOVE_ANTICLOCKWIZE(void)
{
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//机器人左前行_4
void MOTOR_MOVE_TURNLEFT(u8 m_move_speed)
{
	u8 m_move_speed_l_tmp;

	m_move_speed_l_tmp = m_move_speed - 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed_l_tmp);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed);
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//机器人右前行_5
void MOTOR_MOVE_TURNRIGHT(u8 m_move_speed)
{
	u8 m_move_speed_r_tmp;

	m_move_speed_r_tmp = m_move_speed - 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed_r_tmp);
	M_MOVE_DIREC_L 	= 	M_MOVE_Forward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Forward;
}

//机器人左后退_6
void MOTOR_MOVE_BACKLEFT(u8 m_move_speed){
	u8 m_move_speed_r_tmp;

	m_move_speed_r_tmp = m_move_speed + 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed_r_tmp);
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;	
}

//机器人右后退_7
void MOTOR_MOVE_BACKRIGHT(u8 m_move_speed){
	u8 m_move_speed_l_tmp;

	m_move_speed_l_tmp = m_move_speed + 10;

	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed_l_tmp);
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed);
	M_MOVE_DIREC_L 	= 	M_MOVE_Backward;
	M_MOVE_DIREC_R 	= 	M_MOVE_Backward;
}

//机器人停止_8
void MOTOR_MOVE_STOP(void)
{
	TIM_SetCompare2(TIM8, 0);
	TIM_SetCompare3(TIM8, 0);
	M_MOVE_STOP_L 	= 	M_MOVE_Stop;
	M_MOVE_STOP_R 	= 	M_MOVE_Stop;
}

//机器人运行_9
void MOTOR_MOVE_RUN(u8 m_move_speed)
{
	TIM_SetCompare2(TIM8, m_move_speed);
	TIM_SetCompare3(TIM8, m_move_speed);
	M_MOVE_STOP_L 	= 	M_MOVE_Run;
	M_MOVE_STOP_R 	= 	M_MOVE_Run;	
}

//机器人行走控制函数：
//run_flag->0运行,1停止;
//direc_flag->0前进,1后退,2顺时针旋转,3逆时针旋转, 4左前行, 5右前行, 6左后退 , 7右后退;
//speed->左右行走电机速度
void MOTOR_MOVE(u8 m_move_direc, u16 m_move_speed)
{
	MOTOR_MOVE_SPEED_Set(M_MOVE_L, m_move_speed);		//设置左行走电机转速
	MOTOR_MOVE_SPEED_Set(M_MOVE_R, m_move_speed);		//设置右行走电机转速

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

u16 MOTOR_MOVE_DIST_L_READ(void)												//左行走电机里程计数值读取函数
{
	u16 motor_move_dist_val = 0;
	motor_move_dist_val = TIM_GetCounter(TIM4);
	return(motor_move_dist_val);
}

u16 MOTOR_MOVE_DIST_R_READ(void)												//右行走电机里程计数值读取函数
{
	u16 motor_move_dist_val = 0;
	motor_move_dist_val = TIM_GetCounter(TIM3);
	return(motor_move_dist_val);
}

//行走电机避障动作函数
//后退->顺时针旋转m_move_clkwiz_angle
void MOTOR_MOVE_AVOID(u16 m_move_backward_angle, u16 m_move_turn_angle, u16 m_move_direc, u16 m_move_speed)
{
	MOTOR_TURN_Angle(m_move_backward_angle, M_MOVE_Backward, m_move_speed);
	MOTOR_TURN_Angle(m_move_turn_angle, m_move_direc, m_move_speed);
}

//机器人行走电机软启动函数,使用定时器
//m_motor_run->0停止,1运行;
//m_motor_speed_max->风机最终需达到的速度
//m_motor_time->软启动总时间,最小1s
//void MOTOR_MOVE_SOFT_START(u8 m_motor_run, u16 m_motor_speed_max, u16 m_motor_time)
//{
//	u16 m_motor_speed_temp;
//	if(m_motor_run == 0){																													//软启动开始
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
//		TIM_SetCompare2(TIM8,0);																											//左行走电机停止
//		TIM_SetCompare3(TIM8,0);																											//右行走电机停止
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

//控制行走电机转向m_move_angle度的函数
	//m_move_length_r->行走轮半径,全局变量
	//m_move_length_l->行走轮中心到机器人中心的距离,全局变量
	//m_move_encoder_num->行走轮转一圈对应码盘计数值,全局变量
	//m_move_angle->机器人转向角度
	//m_move_direc->机器人转向方向
void MOTOR_TURN_Angle(u16 m_move_angle, u16 m_move_direc, u16 m_move_speed)
{
	u32 waiting_time = 0;
	u16 encoder_target_num = 0;
	u32 calc_tmp_num = 0;																													//用于计算中间量的临时变量
	u16 encoder_tim3_cnt = 0;
	u16 encoder_tim4_cnt = 0;

	TIM_SetCompare2(TIM8,m_move_speed);																						//设置左行走电机转速
	TIM_SetCompare3(TIM8,m_move_speed);																						//设置右行走电机转速

	calc_tmp_num = m_move_angle*m_move_length_l*m_move_encoder_num;
	encoder_target_num = (u16)(calc_tmp_num /(360*m_move_length_r));
	
	TIM_SetCounter(TIM3, 0);																											//清右码盘计数值
	TIM_SetCounter(TIM4, 0);																											//清左码盘计数值

	switch(m_move_direc){
		case(1):																																			//后退,根据给定angle不同后退不同距离,后退距离=(angle*pi*length_l/180)
			MOTOR_MOVE_BACKWARD();
			break;
		case(2):																																			//顺时针
			MOTOR_MOVE_CLOCKWIZE();
			break;
		case(3):																																			//逆时针
			MOTOR_MOVE_ANTICLOCKWIZE();
			break;
		case(4):																																			//左前行
			MOTOR_MOVE_TURNLEFT(m_move_speed);
			break;
		case(5):																																			//右前行
			MOTOR_MOVE_TURNRIGHT(m_move_speed);
			break;
		case(6):																																			//左后退
			MOTOR_MOVE_BACKLEFT(m_move_speed);
			break;
		case(7):																																			//右后退
			MOTOR_MOVE_BACKRIGHT(m_move_speed);
			break;
		default:																																			//顺时针
			MOTOR_MOVE_CLOCKWIZE();
			break;
	}

	for(waiting_time = 0x300000;waiting_time>0;waiting_time--){											//等待码盘计数值达到转angle角度需要的计数值,若超时则自动退出
		if((encoder_tim3_cnt < encoder_target_num)&&(encoder_tim4_cnt < encoder_target_num)){
			encoder_tim3_cnt = TIM_GetCounter(TIM3);
			encoder_tim4_cnt = TIM_GetCounter(TIM4);
		}else
			break;
	}
	printf("target=%d,wait=%d,tim3cnt=%d,tim4cnt=%d\r\n",encoder_target_num,waiting_time,encoder_tim3_cnt,encoder_tim4_cnt);
}
