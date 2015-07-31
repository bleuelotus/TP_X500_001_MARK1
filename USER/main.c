//**************************************************************************************************************//
//Title:TP_X500_001_MARK1																																												//
//Author:Xluo																																																		//
//Corp.:EJE																																																			//
//Data:2015.07.20																																																//
//Version: 	V1.0				Create      																																2015/07/20	//
//         	V1.1				Add Status_Change_Loop function																							2015/07/24	//
//				 	V1.2				Add Plan_Loop function																											2015/07/28	//
//				 	V1.3				Change remote.c,Add return function																					2015/07/29	//
//					V1.4				Change ROB_Sub_Status from Struct to Union																	2015/07/30	//
//**************************************************************************************************************//

#include "usr_includes.h"

#include "usart.h"
#include "sensor.h"
#include "motor_move.h"
#include "motor_side.h"
#include "motor_fan.h"
#include "led.h"
#include "remote.h"
#include "key.h"
#include "voice.h"
#include "power5v.h"
#include "charger.h"
#include "timer.h"

//机器人基本状态参数
static u8	ROB_Basic_Status_flag = 0;																					//机器人基本状态标识,1->待机,2->打扫,3->自动返回充电座,4->充电,5->休眠,6->报警

//机器人子状态相关参数
typedef union{
	u16 ROB_Sub_Status;
	struct{
		u8 sys_err_flag:1;																														//bit0	,系统有异常标识
		u8 battery_Low_flag:1;																												//bit1	,电池电量低标识
		u8 battery_very_low_flag:1;																										//bit2	,电池电量极低标识
		u8 battery_temp_high_flag:1;																									//bit3	,电池温度高标识
		u8 motor_overload_flag:1;																											//bit4	,电机过载标识
		u8 wheel_up_err_flag:1;																												//bit5	,双轮抬起标识
		u8 garbage_box_err_flag:1;																										//bit6	,尘盒未放标识
		u8 garbage_box_full_flag:1;																										//bit7	,尘盒满标识
		u8 power_connected_flag:1;																										//bit8	,接通24V充电电源标识
		u8 charge_alignment_en_flag:1;																								//bit9	,开始与充电座对接标识
		u8 charge_start_flag:1;																												//bit10	,充电开始标识
		u8 battery_full_flag:1;																												//bit11	,电量已充满标识
		u8 charge_finished_flag:1;																										//bit12	,充电完成标识
		u8 plan_en_flag:1;																														//bit13	,开始设置计划任务标识
		u8 plan_cnt_start_flag:1;																											//bit14	,计划任务倒计时开始标识
		u8 plan_time_up_flag:1;																												//bit15	,计划任务时间到标识
	}ROB_Sub_Status_bit;
}ROB_Sub_Status_Union_Def;
ROB_Sub_Status_Union_Def ROB_Sub_Status_Union;

//传感器相关参数
static u16 sensor_val[27];																										//存储传感器采集数据的数组

//传感器->避障相关参数
static union{
	u8 Sens_Avoid_Front_Value;
	struct{
		u8 sens_avoid_front_r_value:1;																								//bit0	,前避障左
		u8 sens_avoid_front_rm_value:1;																								//bit1	,前避障左中
		u8 sens_avoid_front_m_value:1;																								//bit2	,前避障中
		u8 sens_avoid_front_lm_value:1;																								//bit3	,前避障右中
		u8 sens_avoid_front_l_value:1;																								//bit4	,前避障右
	}Sens_Avoid_Front_Value_bit;
}Sens_Avoid_Front_Value_Union;

//传感器->前碰撞相关参数
static union{
	u8 Sens_Avoid_Crush_Value;
	struct{
		u8 sens_avoid_crash_l_value:1;																								//bit0	,前碰撞左
		u8 sens_avoid_crash_r_value:1;																								//bit1	,前碰撞右
	}Sens_Avoid_Crush_Value_bit;
}Sens_Avoid_Crush_Value_Union;

//传感器->探底相关参数
static union{
	u8 Sens_Avoid_Bottom_Value;
	struct{
		u8 sens_avoid_bottom_l_value:1;																								//bit0	,探底左
		u8 sens_avoid_bottom_lm_value:1;																							//bit1	,探底左中
		u8 sens_avoid_bottom_rm_value:1;																							//bit2	,探底右中
		u8 sens_avoid_bottom_r_value:1;																								//bit3	,探底右
	}Sens_Avoid_Bottom_Value_bit;
}Sens_Avoid_Bottom_Value_Union;

////传感器->充电座定位相关参数
//static union{
//	u8 Sens_Avoid_Charge_Box_Location_Value;
//	struct{
//		u8 sens_charge_box_location_l_value:1;																				//bit0	,充电座定位左
//		u8 sens_charge_box_location_lm_value:1;																				//bit1	,充电座定位左中
//		u8 sens_charge_box_location_rm_value:1;																				//bit2	,充电座定位右中
//		u8 sens_charge_box_location_r_value:1;																				//bit3	,充电座定位右
//	}Sens_Avoid_Charge_Box_Location_Value_bit;
//}Sens_Avoid_Charge_Box_Location_Value_Union;

//LED显示屏参数,全局变量
u8 led_code[]		={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};										//用于LED显示屏屏显的数组,第一字节无效

//电容按键相关参数
static u8 key_keysta = 0;																											//存放当前键值
static u8 key_exti_enter_flag = 0;																						//进入按键中断标识,退出中断后在按键处理函数中清零

//行走电机参数
static u8 m_move_speed 	= 25;																									//行走电机速度,用于设置TIM8_CH2&TIM8_CH3 COMPARE值以调整PWM占空比,20%
//static u8 sens_ir_led_brightness = 42;																				//避障发射LED强度值,占空比42%
static u8 m_fan_speed		= 30;																									//风机速度,用于设置TIM8_CH1 COMPARE值以调整PWM占空比,30%

//行走电机相关物理常量,全局变量
const u16 m_move_length_r     		= 30;																				//行走轮半径
const u16 m_move_length_l     		= 150;																			//行走轮中心到机器人中心的距离
const u16 m_move_encoder_num			= 900;																			//行走轮转一圈对应码盘计数值

//行走电机_机器人旋转角度常量
//static const u16 m_move_ledge_l_angle 	= 30;																	//左边轮与左避障传感器夹角
//static const u16 m_move_l_lm_angle 			= 30;																	//左避障传感器与左中避障传感器夹角
//static const u16 m_move_lm_m_angle 			= 30;																	//左中避障传感器与中避障传感器夹角
//static const u16 m_move_m_rm_angle 			= 30;																	//中避障传感器与右中避障传感器夹角
//static const u16 m_move_rm_r_angle			= 30;																	//右中避障传感器与右避障传感器夹角
//static const u16 m_move_r_redge_angle 	= 90;																	//右避障传感器与右边轮夹角
//static const u16 m_move_redge_m_angle 	= 90;																	//右边轮与中避障传感器夹角
static const u16 m_move_ledge_m_angle 	= 90;																	//左边轮与中避障传感器夹角
static const u16 m_move_backward_angle 	= 90;																	//机器人后退用的角度值,可换算成距离
static const u16 m_move_turn_around_angle = 180;															//机器人掉头用角度值
static u16 m_move_usr_angle = 30;																							//行走电机转向角度变量

//机器人转m_move_usr_angle角度,码盘应计数到的值:encoder_target_num = (usr_angle*length_l*encoder_num)/(360*length_r)

//语音芯片相关参数
static u8	voice_play_times_charging_start_ch = 1;															//"开始充电"语音播放次数,初始化为播放次数值,使用时递减直到为0,为0后语音不再播放
static u8	voice_play_times_charging_finished_ch = 1;													//"电量已充满"语音播放次数

//充电相关参数,全局变量
u8 g_charge_led_horse_lamp_cnt = 0;
u8 g_charge_start_flag = 0;

//充电PWM相关参数
static u8 charge_pwm_pulse_width = 0;

//TIM5定时参数
static u8 timer5_500ms_flag = 0;																							//定时器计时到500ms标识
static u8 timer5_1s_charging_led_flag	= 0;																		//定时器计时到1s标识,用于充电时LED流水灯闪烁延时
static u8 timer5_1s_switch_err_flag = 0;																			//定时器计时到1s标识,用于充电时电源开关未开错误提示音延时
static u8 timer5_1s_charging_1d4_flag = 0;																		//定时器计时到1s标识,用于充电时电池电量低于1/4满电电量时的充电占空比变化延时
//static u8 timer5_1s_motor_fan_flag = 0;																				//定时器计时到1s标识,用于风机软启动延时
static u8 timer5_1s_Dush_Full_flag = 0;																				//定时器计时到1s标识,尘盒满错误提示音延时
static u8 timer5_1s_battery_very_low_flag = 0;																//定时器计时到1s标识,电池电量极低错误提示音延时
static u8 timer5_1min_flag = 0;																								//定时到1分钟标识
//static u8 timer5_1hour_flag = 0;																							//定时到1小时标识,用于计划任务小时倒计时
static u8 timer5_24hour_flag = 0;																							//定时到24小时标识,用于计划任务每天相同时间重复倒计时
static u8 timer5_cnt_10ms = 0;
static u8 timer5_cnt_100ms = 0;
static u8 timer5_cnt_500ms = 0;
static u8 timer5_cnt_1s = 0;
static u8 timer5_cnt_1min = 0;
static u8 timer5_cnt_30min = 0;
static u8 timer5_cnt_1hour = 0;
static u8 timer5_cnt_24hour = 0;

//计划任务相关参数
static u8 plan_usr_min_value 	= 0;																						//用户设置的计划任务倒计时时间->分钟
static u8 plan_usr_hour_value 	= 0;																					//用户设置的计划任务倒计时时间->小时
static u8 plan_cnt_min_value = 0;																							//计划任务倒计时时间->分钟
static u8 plan_cnt_hour_value = 0;																						//计划任务倒计时时间->小时
static u8 plan_step_change_flag = 0;																					//计划任务设置步进值切换标识,1对应设置步进值为10,0对应设置步进值为1;
//static u8 plan_24hour_time_up_flag = 0;																				//24小时倒计时时间到标识

//系统初始化
void Usr_System_Init(void)
{
	delay_init();	    	 																													//延时函数初始化
	NVIC_Configuration(); 	 																											//设置NVIC中断分组2:2位抢占优先级,2位响应优先级
	UART_init(9600);	 																														//串口初始化,波特率9600
	TIM5_Int_Init(999,719);																												//TIM5初始化,用作10ms定时基准
	LED_Init();																																		//LED显示屏初始化
	KEY_Init();																																		//电容按键初始化
	REMOTE_Init();																																//红外接收端口初始化
	VOICE_Init();																																	//语音芯片端口初始化
	POWER5V_Init();																																//5V电源控制端口初始化
	SENSOR_Init();																																//传感器端口初始化
 	MOTOR_SIDE_Init();		  																											//边刷电机端口初始化
  MOTOR_FAN_Init();          																										//吸尘风机端口初始化
	MOTOR_MOVE_Init();																														//行走电机端口初始化
	TIM_CtrlPWMOutputs(TIM8, ENABLE);																							//使能TIM8_PWM发生器,使能行走电机,使能风机,使能避障发射
	CHARGER_Init();																																//充电控制端口初始化

//	VOICE_PLAY(VOICE_Play_remind_long);																						//初始化完成提示音
	printf("System_Init finished!\r\n");																					//初始化成功
}

//更新LED显示屏数据前首先将LED显示数组清零,防止更新数据时保留之前的显示
void Disp_Loop(void)
{
	u8 i;

	for(i=0;i<8;i++)
		led_code[i] = 0x00;

	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){											//若设置计划任务标识置位,则显示倒计时设置时间
		LED_DISP_Time(led_code, plan_usr_hour_value, plan_usr_min_value);
	}
}

//电容按键中断处理函数
void EXTI9_5_IRQHandler(void)
{
	key_exti_enter_flag = 1;
	if(EXTI_GetFlagStatus(EXTI_Line5) != RESET){
		key_keysta = KEY_SPOT;																													//SPOT按下
		ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop,M_FAN_speed_stop);
	}else if(EXTI_GetFlagStatus(EXTI_Line6) != RESET){
		key_keysta = KEY_CLEAN;																													//CLEAN按下
	}else if(EXTI_GetFlagStatus(EXTI_Line7) != RESET){
		key_keysta = KEY_PLAN;																													//PLAN按下
	}else if(EXTI_GetFlagStatus(EXTI_Line8) != RESET){
		key_keysta = KEY_HOME;																													//HOME按下
	}else
		key_keysta = 0;
	EXTI->PR  |= (1<<5)|(1<<6)|(1<<7)|(1<<8);																			//清除LINE5,6,7,8上的中断标识
}

//电容按键键值处理函数
void Key_Loop(void)
{
	if(key_exti_enter_flag){
		switch(key_keysta){
			case(KEY_SPOT):																														//SPOT键按下
				LED_DISP(led_code, LED_SPOT, 1);																							//SPOT灯亮起
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){																			//当进入充电状态时,SPOT键无效

				}else if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){																		//当进入计划任务设置状态时,SPOT按键用于增加计划任务时间,步进1min
					plan_step_change_flag = 0;																								//步进值设置为1
					if(plan_usr_min_value<60){
						plan_usr_min_value += 1;
					}else{
						plan_usr_hour_value += 1;
						plan_usr_min_value = 0;
					}
				}else{
					MOTOR_MOVE_STOP();
					MOTOR_SIDE(M_SIDE_Stop);
					MOTOR_FAN(M_FAN_Stop,M_FAN_speed_stop);
					ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;
				}
				break;
			case(KEY_PLAN):																														//PLAN键按下
				LED_DISP(led_code, LED_PLAN, 1);																							//PLAN灯亮起
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){
					if(plan_usr_hour_value||plan_usr_min_value){
						plan_cnt_min_value 	= 0;																									//当用户设置倒计时时间有值,并确认设置时,倒计时分钟数清零
						plan_cnt_hour_value = 0;																									//当用户设置倒计时时间有值,并确认设置时,倒计时分钟数清零
						timer5_24hour_flag = 0;																										//当用户设置倒计时时间有值,并确认设置时,24小时计时到标识清零
						timer5_cnt_1min = 0;																											//当用户设置倒计时时间有值,并确认设置时,1分钟倒计时计数值清零
						timer5_cnt_1hour = 0;																											//当用户设置倒计时时间有值,并确认设置时,1小时倒计时计数值清零
						timer5_cnt_24hour = 0;																										//当用户设置倒计时时间有值,并确认设置时,24小时倒计时计数值清零
						ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag = 1;					//当用户设置倒计时时间有值,并确认设置时,计划任务倒计时开始标识置位
						VOICE_PLAY(VOICE_Play_set_success);																				//当用户设置倒计时时间有值时,提示设置成功
					}
					ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag = 0;									//设置计划任务使能标识清零
				}else{
					ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag = 1;									//设置计划任务使能标识置位
				}
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;														//切换到待机状态
				break;
			case(KEY_CLEAN):																													//CLEAN键按下
				LED_DISP(led_code, LED_CLEAN,	1);																					//CLEAN灯亮起
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){									//当进入计划任务设置状态时,CLEAN键用于减少计划任务时间,步进1min
					if(plan_usr_min_value){
						if(plan_step_change_flag)
							plan_usr_min_value -= 10;
						else
							plan_usr_min_value -= 1;
					}else{
						if(plan_usr_hour_value){
							plan_usr_min_value = 60;
							plan_usr_hour_value -=1;
						}else{
							plan_usr_min_value = 0;
							plan_usr_hour_value = 0;
						}
					}
				}else{
					ROB_Basic_Status_flag = ROB_Basic_Cleaning_flag;													//切换到打扫状态
				}
				break;
			case(KEY_HOME):																														//HOME键按下
				LED_DISP(led_code, LED_HOME, 1);																					//HOME灯亮起
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){
					plan_step_change_flag = 1;																								//步进值设置为10
					if(plan_usr_min_value<60){
						plan_usr_min_value += 10;
					}else{
						plan_usr_hour_value += 1;
						plan_usr_min_value = 0;
					}
				}else{

				}
				//ROB_Basic_Status_flag = ROB_Basic_Returning_flag;														//切换到自动充电状态
				break;
			default:

				break;
		}
		key_exti_enter_flag = 0;
	}
}

//红外遥控接收处理函数
void Remote_loop(void)
{
	u8 Rmtkey = 0;																																//红外遥控接收到的键值

	Rmtkey=REMOTE_Scan();																													//红外遥控接收键值扫描

	if(Rmtkey){
		switch(Rmtkey){
			case 0:																																				//str="ERROR";

				break;
			case 162:																																			//str="POWER";
				if(ROB_Basic_Status_flag == ROB_Basic_Cleaning_flag)
					ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若在打扫状态,切换到待机状态
				else
					ROB_Basic_Status_flag = ROB_Basic_Cleaning_flag;															//若不在打扫状态,切换到打扫状态
				break;
			case 98:																																			//str="UP";
				MOTOR_MOVE(M_MOVE_Forward,m_move_speed);
				break;
			case 2:																																				//str="PLAY";
				break;
			case 226:																																			//str="ALIENTEK";
				if(ROB_Basic_Status_flag == ROB_Basic_Cleaning_flag)
					MOTOR_SIDE(M_SIDE_Run);
				else
					MOTOR_SIDE(M_SIDE_Stop);
				break;
			case 194:																																			//str="RIGHT";
				MOTOR_MOVE(M_MOVE_ClkWiz,m_move_speed);
				break;
			case 34:																																			//str="LEFT";
				MOTOR_MOVE(M_MOVE_AntiClkWiz,m_move_speed);
				break;
			case 224:																																			//str="VOL-";
				if(m_move_speed > 0){
					m_move_speed -= 1;
//				sens_ir_led_brightness -= 1;
				}else{
					m_move_speed = 0;
//				sens_ir_led_brightness = 0;
				}
//				printf("m_move_speed=%d\r\n",m_move_speed);
				TIM_SetCompare2(TIM8,m_move_speed);																						//设置左行走电机转速
				TIM_SetCompare3(TIM8,m_move_speed);																						//设置右行走电机转速
//				TIM_SetCompare4(TIM8,sens_ir_led_brightness);																					//设置避障发射LED发射强度
				break;
			case 168:																																			//str="DOWN";
				MOTOR_MOVE(M_MOVE_Backward,m_move_speed);
//			printf("MOTOR_MOVE_Backward\r\n");
				break;
			case 144:																																			//str="VOL+";
				if(m_move_speed < M_MOVE_speed_h){
					m_move_speed += 1;
//				sens_ir_led_brightness += 1;
				}else{
					m_move_speed = M_MOVE_speed_h;
//				sens_ir_led_brightness = SENS_LED_Value_max;
				}
				printf("m_move_speed=%d\r\n",m_move_speed);
				TIM_SetCompare2(TIM8,m_move_speed);																						//设置左行走电机转速
				TIM_SetCompare3(TIM8,m_move_speed);																						//设置右行走电机转速
//			TIM_SetCompare4(TIM8,sens_ir_led_brightness);																						//设置避障发射LED发射强度
				break;
			case 104:																																			//str="1";
				LED_DISP(led_code,LED_NUM1,1);
				MOTOR_MOVE_TURNLEFT(m_move_speed);
//				if(m_move_usr_angle > 0)
//					m_move_usr_angle -= 1;
//				else
//					m_move_usr_angle = 0;
//				printf("usr_angle=%d\r\n",m_move_usr_angle);
				break;
			case 152:																																			//str="2";
				LED_DISP(led_code,LED_NUM1,2);
				break;
			case 176:																																			//str="3";
				LED_DISP(led_code,LED_NUM1,3);
				MOTOR_MOVE_TURNRIGHT(m_move_speed);
//				if(m_move_usr_angle <180)
//					m_move_usr_angle += 1;
//				else
//					m_move_usr_angle = 180;
//				printf("usr_angle=%d\r\n",m_move_usr_angle);
				break;
			case 48:																																			//str="4";
				LED_DISP(led_code,LED_NUM1,4);
				MOTOR_MOVE(M_MOVE_TurnLeft,m_move_speed);
				break;
			case 24:																																			//str="5";
				LED_DISP(led_code,LED_NUM1,5);
				break;
			case 122:																																			//str="6";
				LED_DISP(led_code,LED_NUM1,6);
				MOTOR_MOVE(M_MOVE_TurnRight,m_move_speed);
				break;
			case 16:																																			//str="7";
				LED_DISP(led_code,LED_NUM1,7);
				MOTOR_TURN_Angle(m_move_usr_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case 56:																																			//str="8";
				LED_DISP(led_code,LED_NUM1,8);
				break;
			case 90:																																			//str="9";
				LED_DISP(led_code,LED_NUM1,9);
				MOTOR_TURN_Angle(m_move_usr_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case 66:																																			//str="0";
				LED_DISP(led_code,LED_NUM1,0);
				break;
			case 82:																																			//str="DELETE";
				MOTOR_MOVE_SOFT_START_SIMPLE();
				break;
			default:

				break;
		}
	}
	Rmtkey = 0;
}

//传感器状态读取函数
void Sensor_Loop(void)
{
	u8 i;

	for(i=0;i<=6;i++)			sensor_val[i] = SENSOR_Scan(i);													//sensor[0..6]前避障传感器和前避障碰撞传感器数据读取

	for(i=7;i<=10;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[7..10]探底传感器数据读取

	for(i=11;i<=13;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[11..13]故障状态传感器数据读取(双轮抬起,垃圾盒未放检测,灰尘过多检测)

	//for(i=14;i<=18;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[14..18]充电座自充电相关传感器数据读取,不能直接读IO电平,需要在remote.c中处理

	for(i=19;i<=23;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[19..23]过载传感器数据读取

	for(i=24;i<=26;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[24..26]电池电量检测,充电时电源电量检测,电池温度检测

	//printf("ROB_Sub_Status=%d\r\n",ROB_Sub_Status_Union.ROB_Sub_Status);					//打印系统状态寄存器值
	if(timer5_500ms_flag){
		for(i=0;i<=10;i++)		printf("[%d]=%d,",i,sensor_val[i]);											//传感器数组值打印,调试用
		printf("\r\n");
		timer5_500ms_flag = 0;
	}

	//将避障导航传感器读取值赋值给传感器共用体，供导航函数使用
	if(SENS_F_L_nEN)
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_l_value = 1;
	else
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_l_value = 0;
	if(SENS_F_LM_nEN)
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_lm_value = 1;
	else
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_lm_value = 0;
	if(SENS_F_M_nEN)
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_m_value = 1;
	else
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_m_value = 0;
	if(SENS_F_RM_nEN)
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_rm_value = 1;
	else
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_rm_value = 0;
	if(SENS_F_R_nEN)
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_r_value = 1;
	else
		Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value_bit.sens_avoid_front_r_value = 0;
	if(SENS_CRUSH_L_nEN)
		Sens_Avoid_Crush_Value_Union.Sens_Avoid_Crush_Value_bit.sens_avoid_crash_l_value = 1;
	else
		Sens_Avoid_Crush_Value_Union.Sens_Avoid_Crush_Value_bit.sens_avoid_crash_l_value = 0;
	if(SENS_CRUSH_R_nEN)
		Sens_Avoid_Crush_Value_Union.Sens_Avoid_Crush_Value_bit.sens_avoid_crash_r_value = 1;
	else
		Sens_Avoid_Crush_Value_Union.Sens_Avoid_Crush_Value_bit.sens_avoid_crash_r_value = 0;
	if(SENS_B_L_nEN)
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_l_value = 1;
	else
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_l_value = 0;
	if(SENS_B_LM_nEN)
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_lm_value = 1;
	else
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_lm_value = 0;
	if(SENS_B_RM_nEN)
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_rm_value = 1;
	else
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_rm_value = 0;
	if(SENS_B_R_nEN)
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_r_value = 1;
	else
		Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value_bit.sens_avoid_bottom_r_value = 0;
}

//避障及自动返回导航函数
void Navigation_Loop(u8 nav_avoid_en_flag, u8 nav_return_en_flag)
{
	if(nav_avoid_en_flag){
		//探底传感器优先级最高,4个传感器共组合出16种状态,调试时判断状态与实际相反，需更改"usr_include.h"中的宏定义
		switch(Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value){
			case(0):	//			printf("探底传感器L,LM,RM,R同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(1):	//			printf("探底传感器L,LM,RM同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(2):	//			printf("探底传感器L,RM,R同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(3):	//			printf("探底传感器RM,R同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(4):	//			printf("探底传感器L,LM,R同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(5):	//			printf("探底传感器LM,R同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(6):	//			printf("探底传感器L,R同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(7):	//			printf("探底传感器R触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(8):	//			printf("探底传感器L,LM,RM同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(9):	//			printf("探底传感器LM,RM同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(10):	//			printf("探底传感器L,RM同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(11):	//			printf("探底传感器L,LM,R同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(12):	//			printf("探底传感器L,LM同时触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(13):	//			printf("探底传感器LM触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(14):	//			printf("探底传感器L触发!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			default:	//			printf("探底传感器未触发!\r\n");

				break;
		}

		//探底传感器未触发,碰撞传感器优先级高,2个传感器共组合出4种状态,碰撞传感器判断依据调试与实际一致
		switch(Sens_Avoid_Crush_Value_Union.Sens_Avoid_Crush_Value){
			case(0):	//				printf("前方碰撞障碍物!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(1):	//				printf("左方碰撞障碍物!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(2):	//				printf("右方碰撞障碍物!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			default:	//				printf("未发生碰撞!\r\n");

				break;
		}

		//碰撞传感器未触发,使用避障传感器进行导航,5个传感器共组合出32种状态
		switch(Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value){
			case(0):	//				printf("前方及两侧均有墙角!\r\n");															0x00->00000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(1):	//				printf("右前方有墙角,左侧方有通道!");														0x01->00001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(2):	//				printf("右前方有墙角,左侧方有障碍物!\r\n");											0x02->00010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(3):	//				printf("右前方有墙角,左侧方可通过!\r\n");												0x03->00011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(4):	//				printf("前方及两侧均有障碍物!\r\n");														0x04->00100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(5):	//				printf("右方有障碍物,左侧有可绕过障碍物!\r\n");									0x05->00101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(6):	//				printf("右方有障碍物,左侧有不能通过的通道!\r\n");								0x06->00110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(7):	//				printf("右侧方有障碍物,左前方可通过!\r\n");											0x07->00111
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(8):	//				printf("右前方有不能通过的通道,左前方有墙角!\r\n");							0x08->01000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(9):	//				printf("右前方有不能通过的通道,左侧方可以通过!\r\n");						0x09->01001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(10):	//				printf("左右两侧均有无法通过的通道!\r\n");											0x0A->01010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(11):	//				printf("右前方有无法通过的通道,左方可以通过!\r\n");							0x0B->01011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(12):	//				printf("右前方有无法通过的通道,左侧方有障碍物!\r\n");						0x0C->01100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(13):	//				printf("右前方有无法通过的通道,左侧方可以通过!\r\n");						0x0D->01101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(14):	//				printf("前方有无法通过的通道,左右两侧有障碍物!\r\n");						0x0E->01110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(15):	//				printf("右侧方有障碍物,左前方可以通过,可以贴右侧方墙走!\r\n");	0x0F->01111
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(16):	//				printf("右前方有通道,左侧方有障碍物!\r\n");											0x10->10000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(17):	//				printf("前方有墙,左右两侧有通道!\r\n");													0x11->10001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(18):	//				printf("右侧方有通道,左前方有无法通过的通道!\r\n");							0x12->10010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(19):	//				printf("右侧方有通道,左侧方有可以通过的通道!\r\n");							0x13->10011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(20):	//				printf("右前方有可以从右侧绕过的障碍物,左侧方有障碍物!\r\n");		0x14->10100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(21):	//				printf("右前方与左前方均有可以绕过的障碍物!\r\n");							0x15->10101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(22):	//				printf("右前方有可以绕过的障碍物,左前方有无法通过的通道!\r\n");	0x16->10110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(23):	//				printf("右前方有可以绕过的障碍物,左侧方可通过!\r\n");						0x17->10111
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(24):	//				printf("右侧方有通道,左前方有墙角!\r\n");												0x18->11000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(25):	//				printf("右侧方有通道(通过可能性大),左侧方有通道(通过可能性小)!\r\n");	0x19->11001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(26):	//				printf("前方有可从右侧方绕过的障碍物,左前方有无法通过的通道!\r\n");		0x20->11010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(27):	//				printf("前方有可绕过的障碍物,左右侧通过的可能性一样!\r\n");						0x21->11011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(28):	//				printf("右侧方可通过,左侧方有障碍物!\r\n");														0x22->11100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(29):	//				printf("右侧方可通过,左侧方有可绕过的障碍物!\r\n");										0x23->11101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(30):	//				printf("右前方可通过,左侧方有障碍物!\r\n");														0x24->11110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			default:	//				printf("无障碍\r\n");																									0x25->11111
				MOTOR_MOVE(M_MOVE_Forward,m_move_speed);
				break;
		}
	}else{																																				//避障导航禁能
		if(nav_return_en_flag)																												//若自动返回导航使能,不停止电机
			;
		else{																																					//若导航全部禁能,电机全部停止
			MOTOR_MOVE(M_MOVE_Forward,m_move_speed);
			MOTOR_SIDE(M_SIDE_Stop);
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		}
	}
	if(nav_return_en_flag){																												//若自动返回标识使能,运行自动返回导航,当与充电座已对准进行对接时,应禁能避障功能
		//if(){
			//;
		//}else if(){
			//ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_alignment_en_flag = 0;					//若对准条件丢失，则清除对准标识
		//}else{
			//ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_alignment_en_flag = 1;					//机器人与充电座对齐后,自动对接开始标识置位,此标志置位后应禁能避障功能
		//}
	}else
		;
}

//充电时电量指示灯处理函数,通过LED闪烁状态显示充电电量
void Charging_LED_Horse_Lamp(u8 led_horse_lamp_en)
{
	if(led_horse_lamp_en){																												//若充电开始标识置位,则开始显示充电电量
		if(sensor_val[SENS_Battery] < SYS_Battery_1d4){																//电池电量小于1/4满电状态时,4个数码管LED依次闪亮数字"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(5);
				timer5_1s_charging_led_flag = 0;
			}
		}else if(sensor_val[SENS_Battery] < SYS_Battery_2d4){													//电池电量小于2/4满电状态时,第1数码管常亮"1",其余3个数码管LED依次闪亮数字"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(4);
				timer5_1s_charging_led_flag = 0;
			}
		}else if(sensor_val[SENS_Battery] < SYS_Battery_3d4){													//电池电量小于3/4满电状态时,第1和第2数码管常亮"1",其余2个数码管LED依次闪亮数字"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(3);
				timer5_1s_charging_led_flag = 0;
			}
		}else if(sensor_val[SENS_Battery] < SYS_Battery_Full){												//电池电量小于满电状态时,第1、第2和第3数码管常亮"1",其余1个数码管LED依次闪亮数字"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(2);
				timer5_1s_charging_led_flag = 0;
			}
		}else{																																				//电池电量充满时,4个数码管常亮"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(1);
				timer5_1s_charging_led_flag = 0;
			}else
				;
		}
	}
}

//充电状态处理函数
void Charging_Loop(void)
{
	Charging_LED_Horse_Lamp(1);																										//当充电开始标识置位后,开始进行流水灯显示
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){
		if(sensor_val[SENS_Battery] < SYS_Battery_Full){															//电量未充满
			MOTOR_MOVE_STOP();																														//充电开始后,行走电机停止
			MOTOR_SIDE(M_SIDE_Stop);																											//充电开始后,边刷电机停止
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);																			//充电开始后,风机停止
			if(sensor_val[SENS_Battery] < SYS_Battery_1d4){
				if(timer5_1s_charging_1d4_flag){
					CHARGER_Start_1d4(charge_pwm_pulse_width);																		//电量小于1/4满电电量时,使用占空比渐变充电
					timer5_1s_charging_1d4_flag = 0;
				}
			}else if(sensor_val[SENS_Battery] < SYS_Battery_3d4){
				CHARGER_Start_more_than_1d4();																								//电量大于1/4满电电量小于3/4满电电量时,使用固定占空比充电,占空比45.5%
			}else{
				CHARGER_Startt_more_than_3d4();																								//电量大于3/4满电电量时,使用固定占空比充电,占空比60%
			}
		}else{																																				//电量已充满
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_finished_flag = 1;							//充电完成标识置位
			CHARGER_Stop();																																//充电电压大于电量满阈值后,停止充电
			if(voice_play_times_charging_finished_ch){
				VOICE_PLAY(VOICE_Play_charging_up_ch);																				//电量充满后语音提示一次
				voice_play_times_charging_finished_ch--;
			}else
				voice_play_times_charging_finished_ch = 0;
		}
	}else
		CHARGER_Stop();
}

//报警状态处理函数
void Warning_Loop(void)
{
	MOTOR_MOVE_STOP();
	MOTOR_SIDE(M_SIDE_Stop);
	MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag){
		LED_DISP_ERR(led_code, 1,1);
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag){
		LED_DISP_ERR(led_code, 1,2);
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag){
		LED_DISP_ERR(led_code, 1,3);
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag){
		LED_DISP_ERR(led_code, 1,4);
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag){
		LED_DISP_ERR(led_code, 1,5);
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag){
		LED_DISP_ERR(led_code, 1,8);
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag){
		LED_DISP(led_code,LED_ERR_Bat_Low, 1);
	}
}

//计划任务处理函数
void Plan_Loop(void)
{
	if(timer5_24hour_flag){																												//若24小时计时时间到标识置位,则使能倒计时
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag = 1;
		timer5_24hour_flag = 0;
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag){							//若计划任务倒计时使能,则开始根据定时标识进行计数
		if(timer5_1min_flag){
			if(plan_cnt_min_value < 60)
				plan_cnt_min_value++;
			else{
				plan_cnt_hour_value++;
				plan_cnt_min_value = 0;
			}
			timer5_1min_flag = 0;
		}
	}
	//判断计时时间是否与计划设置时间相同,相同则置位计时时间到标识
	if(plan_usr_hour_value == 0){
		if(plan_usr_min_value == 0){																									//当用户设置计划任务倒计时时间都为0时,不进行倒计时比较
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag = 0;
		}else{																																				//当用户设置计划任务倒计时时间中的分钟值不为0时,进行倒计时比较
			if(plan_cnt_min_value == plan_usr_min_value){
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag = 1;							//当倒计时分钟数与用户设置分钟数一致时,时间到标识置位
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag = 0;
				plan_cnt_min_value = 0;
				VOICE_PLAY(VOICE_Play_remind_short);
			}
		}
	}else{
		if((plan_cnt_min_value == plan_usr_min_value)&&(plan_cnt_hour_value == plan_usr_hour_value)){
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag = 1;
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag = 0;
			plan_cnt_min_value = 0;
			plan_cnt_hour_value = 0;
			VOICE_PLAY(VOICE_Play_remind_short);
		}
	}
}

//子状态处理函数
//根据传感器状态对相应子状态标识进行置位或清零,排在前面的状态会更早检测到
void Sub_Status_Loop(void)
{
	//不充电时电池电量监测,错误代码"Lo"
	if(ROB_Basic_Status_flag != ROB_Basic_Charging_flag){
		if(sensor_val[SENS_Battery] < SYS_Battery_Low){
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag = 1;
			LED_DISP_ERR(led_code, 1,8);																									//电量低后显示错误代码"E18"
			MOTOR_SIDE(M_SIDE_Stop);																											//电量低后停止使用边刷
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);																			//电量低后停止使用风机
			if(sensor_val[SENS_Battery] < SYS_Battery_Very_Low){													//电池电量低于电量极低阈值后的动作
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag = 1;
				LED_DISP(led_code,LED_ERR_Bat_Low, 1);																				//LED屏显示电池电量低提示,"Lo"
				if(timer5_1s_battery_very_low_flag){
					VOICE_PLAY(VOICE_Play_dong);																									//电池电量极低时通过提示音报警
					timer5_1s_battery_very_low_flag = 0;
				}
			}else{
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag = 0;
			}
		}else{
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag = 0;
		}
	}

	//充电监测,若24V接入但未开启电源开关,错误代码"E10"
	if(sensor_val[SENS_24V] > SYS_Power24V_Connected_Value){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.power_connected_flag = 1;
		if(sensor_val[SENS_Battery] < SYS_Power24V_Switch_err_Value){									//当24V电源接通时,若电池电量检测不到时,可判断是电源开关未打开
			LED_DISP_ERR(led_code, 1, 0);																									//LED屏显示错误代码,"E10"
			CHARGER_Stop();
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 0;
			if(timer5_1s_switch_err_flag){
				VOICE_PLAY(VOICE_Play_dong);
				timer5_1s_switch_err_flag = 0;
			}else
				;
		}else if(sensor_val[SENS_Battery] > SYS_Power24V_Switch_off_err_Value){				//当充电开始后,若充电中途关闭电源开关,停止充电再次报警
			LED_DISP_ERR(led_code, 1, 0);																									//LED屏显示错误代码,"E10"
			CHARGER_Stop();
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 0;
		}else{
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 1;								//开始充电标识置位
			if(voice_play_times_charging_start_ch){																				//开始充电语音提示,播放(VOICE_PALY_Times_charging_start_ch)次
				VOICE_PLAY(VOICE_Play_charging_start_ch);
				voice_play_times_charging_start_ch --;
			}else
				voice_play_times_charging_start_ch = 0;
		}
	}else{
		CHARGER_Stop();																																//没有接入24V电源时关闭充电通路
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.power_connected_flag = 0;
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 0;
	}
	g_charge_start_flag = ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag;//用于将标识传递给sensor.c

	//双轮抬起监测,错误代码"E11"(轮询方式,配合双轮抬起外部中断服务程序使用)
	if(sensor_val[SENS_WHEEL_UP] == 0){
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag = 1;								//双轮抬起标识置位
		LED_DISP_ERR(led_code, 1,1);																									//双轮抬起错误,显示错误代码"E11"
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag = 0;
	}

	//过载保护监测,错误代码"E12"
	if((sensor_val[SENS_FAN_OVLOAD])||(sensor_val[SENS_M_SIDE_L_OVLOAD])||(sensor_val[SENS_M_SIDE_R_OVLOAD])||(sensor_val[SENS_M_MOVE_L_OVLOAD])||(sensor_val[SENS_M_MOVE_R_OVLOAD])){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag = 1;
		MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
		LED_DISP_ERR(led_code, 1,2);																									//显示错误代码"E12"
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag = 0;
	}

	//电池温度高状态监测,错误代码"E13"
	if(sensor_val[SENS_Battery_temp] > SYS_Battery_Temp_High){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag = 1;
		LED_DISP_ERR(led_code, 1,3);																											//电池温度高错误提示,显示"E13"
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		VOICE_PLAY(VOICE_Play_ding);																											//电池温度高错误发生时,不断进行声音提示
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag = 0;
	}

	//尘盒未放状态监测,错误代码"E14"
	if(sensor_val[SENS_NO_GARBAGE_BOX] == 0){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag = 1;
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		LED_DISP_ERR(led_code, 1,4);																											//尘盒未放错误提示,"E14"
		LED_DISP(led_code, LED_GARBAGE_WARNING, 1);																				//尘盒LED错误提示,LED_GARBAGE内层LED亮,红色
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag = 0;
	}

	//尘盒满状态监测,错误代码"E15"
	//尘盒未满为0,尘盒已满为1,调试时相反
	if(sensor_val[SENS_DUSH_FULL] == 0){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag = 1;
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		LED_DISP_ERR(led_code, 1,5);																											//尘盒满错误提示,"E15"
		LED_DISP(led_code, LED_GARBAGE_WARNING, 1);																				//尘盒LED错误提示,LED_GARBAGE内层LED亮,红色
		if(timer5_1s_Dush_Full_flag){
			VOICE_PLAY(VOICE_Play_dong);
			timer5_1s_Dush_Full_flag = 0;
		}else
			;
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag = 0;
	}
}

//系统状态切换函数,根据子状态标识进行系统状态自动切换
void Status_Change_Loop(void)
{
	switch(ROB_Basic_Status_flag){
		case(ROB_Basic_Waiting_flag):																									//待机状态下,可能进行的状态切换
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag){
				ROB_Basic_Status_flag = ROB_Basic_Cleaning_flag;															//若计划任务倒计时时间到,切换到打扫状态
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag = 0;
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){
				ROB_Basic_Status_flag = ROB_Basic_Charging_flag;															//若充电开始标识置位,切换到充电状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若电池电量低,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若双轮抬起标识置位,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若尘盒未放标识置位,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若电池电量低于电量极低阈值,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若电池温度高于温度报警阈值,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若尘盒满标识置位,切换到报警状态
			}else
				;
			break;
		case(ROB_Basic_Cleaning_flag):																								//打扫状态下,可能进行的状态切换
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Returning_flag;															//若电池电量低,切换到自动返回状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.sys_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若系统异常标识置位,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若过载标识置位,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若双轮抬起标识置位,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//若尘盒满标识置位,切换到报警状态
			}else
				;
			break;
		case(ROB_Basic_Returning_flag):																								//自动返回状态下,可能进行的状态切换
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若电池电量极低,切换到报警状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若过载标识置位,切换到待机状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若双轮抬起标识置位,切换到待机状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){
				ROB_Basic_Status_flag = ROB_Basic_Charging_flag;															//若充电开始标识置位,切换到充电状态
			}else
				;
			break;
		case(ROB_Basic_Charging_flag):																								//充电状态下,可能进行的状态切换
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_finished_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若充电完成标识置位,切换到待机状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.power_connected_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若充电过程中电源被拔下,切换到待机状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若充电过程中电源开关关闭,切换到待机状态
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若充电过程中电池温度过高,切换到待机状态
			}else
				;
			break;
		case(ROB_Basic_Sleeping_flag):																								//在休眠状态下,可能进行的状态切换

			break;
		case(ROB_Basic_Warning_flag):																									//在报警状态下,可能进行的状态切换
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若双轮抬起标识清零,切换到待机状态
			}
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//若尘盒未放标识清零,切换到待机状态
			}
			break;
		default:																																			//默认系统处于待机状态
			ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;
			break;
	}
}

//系统状态处理函数,不造成状态跳转的标识置位在此处理
void Status_Loop(void)
{
	switch(ROB_Basic_Status_flag){
		case(ROB_Basic_Waiting_flag):																									//进入待机状态
			MOTOR_MOVE_STOP();
			MOTOR_SIDE(M_SIDE_Stop);
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
			Navigation_Loop(0,0);
			break;
		case(ROB_Basic_Cleaning_flag):																								//进入打扫状态
			if(ROB_Sub_Status_Union.ROB_Sub_Status & 0x00FF){
				Navigation_Loop(0,0);																													//若有异常,关闭自动返回导航函数,停止机器人运动
			}
			else{
				MOTOR_MOVE_RUN(m_move_speed);
			//MOTOR_SIDE(M_SIDE_Run);
			//MOTOR_FAN(M_FAN_Run, m_fan_speed);
				Navigation_Loop(1,0);																													//若无异常,调用打扫导航函数
			}
			break;
		case(ROB_Basic_Returning_flag):																								//进入自动返回状态
			if(ROB_Sub_Status_Union.ROB_Sub_Status & 0x00FF){
				Navigation_Loop(0,0);																													//若有异常,关闭自动返回导航函数,停止机器人运动
			}else
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_alignment_en_flag)
					Navigation_Loop(0,1);																													//若充电座已对准标识置位，则关闭避障功能进行对接
				else
					Navigation_Loop(1,1);																													//若充电座未对准，则打开避障功能进行避障返回导航
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag){									//若电量低标识置位，则显示电量低错误代码
					LED_DISP_ERR(led_code, 1,0);
				}
			break;
		case(ROB_Basic_Charging_flag):																								//进入充电状态
			Charging_Loop();																															//调用充电处理函数
			break;
		case(ROB_Basic_Sleeping_flag):																								//进入休眠状态

			break;
		case(ROB_Basic_Warning_flag):																									//进入报警状态
			Warning_Loop();
			break;
		default:																																			//默认在待机状态
			ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;
			break;
	}
}

//MAIN
int main(void)
{
	Usr_System_Init();
	while(1){
		Key_Loop();
		Remote_loop();
		Sensor_Loop();
		Sub_Status_Loop();																														//调用子状态处理函数进行系统监测（可根据不同状态的需要在此函数中使用状态标识屏蔽不需要的监测）
		Status_Change_Loop();
		Status_Loop();
		Plan_Loop();
		Disp_Loop();
	}
	//return 1;
}

//双轮抬起外部中断服务程序,优先关闭电机
void EXTI1_IRQHandler(void)
{
	MOTOR_MOVE_STOP();
	MOTOR_SIDE(M_SIDE_Stop);
	MOTOR_FAN(M_FAN_Stop, m_fan_speed);
	ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag = 1;
	EXTI->PR &= ~(1<<1);
}

//定时器5中断服务程序,用于产生各种时间标识作为定时基准
void TIM5_IRQHandler(void)																										//TIM5中断
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  													//检查TIM5更新中断发生与否
	{
		timer5_cnt_10ms++;
		timer5_cnt_100ms++;
		timer5_cnt_500ms++;
		timer5_cnt_1s++;
		if(timer5_cnt_500ms == 50){
			timer5_500ms_flag = 1;
			timer5_cnt_500ms = 0;
		}
		if(timer5_cnt_1s == 60){
			timer5_cnt_1min++;
			timer5_1s_charging_led_flag = 1;
			timer5_1s_switch_err_flag = 1;
			timer5_1s_charging_1d4_flag = 1;
//			timer5_1s_motor_fan_flag = 1;
			timer5_1s_Dush_Full_flag = 1;
			timer5_cnt_1s = 0;
		}
		if(timer5_cnt_1min == 60){
			timer5_cnt_30min++;
			timer5_cnt_1hour++;
			timer5_1min_flag = 1;
			timer5_cnt_1min = 0;
		}
		if(timer5_cnt_1hour == 60){
			timer5_cnt_24hour++;
//			timer5_1hour_flag = 1;
			timer5_cnt_1hour = 0;
		}
		if(timer5_cnt_24hour == 24){
			timer5_24hour_flag = 1;
			timer5_cnt_24hour = 0;
		}
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);  																	//清除TIM5更新中断标志
	}
}
