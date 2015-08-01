#include "delay.h"
#include "sys.h"

#include "stdio.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_it.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"

//机器人基本状态常量
#define ROB_Basic_Waiting_flag  					1																		//待机状态标识
#define ROB_Basic_Cleaning_flag 					2																		//打扫状态标识
#define ROB_Basic_Returning_flag 					3																		//自动返回充电座状态标识
#define ROB_Basic_Charging_flag						4																		//充电状态标识
#define ROB_Basic_Sleeping_flag 					5																		//休眠状态标识
#define ROB_Basic_Warning_flag						6																		//报警状态标识

//PWM相关常量
#define TIM8_PWM_ARR_Value								99																	//99->{(99+1)*(71+1)/72000000 = 0.0001s(10Khz)},999->{(999+1)*(71+1)/72000000 = 0.001s(1Khz)}
#define TIM8_PWM_PSC_Value								71

//避障发射LED强度值
#define	SENS_LED_Value_const							42																	//避障发射LED强度值,设置为42左右时亮度最大
#define	SENS_LED_Value_max 								90																	//避障发射LED强度最大值

//行走电机行动方向常量
#define M_MOVE_Forward										0
#define M_MOVE_Backward										1
#define M_MOVE_ClkWiz											2
#define	M_MOVE_AntiClkWiz									3
#define M_MOVE_TurnLeft										4
#define	M_MOVE_TurnRight									5

//行走电机启停常量
#define M_MOVE_Run												0
#define	M_MOVE_Stop												1

//行走电机速度常量
#define M_MOVE_speed_h										70
#define M_MOVE_speed_m										40
#define M_MOVE_speed_l										10

//行走电机控制端口宏_输出
#define M_MOVE_DIREC_L 										PEout(1)														//PE1	->左电机正反转控制,	0正转,1反转
#define M_MOVE_STOP_L 										PEout(2)														//PE2	->左电机启停控制,		0启动,1停止
#define M_MOVE_DIREC_R 										PEout(9)														//PE9	->右电机正反转控制,	0正转,1反转
#define M_MOVE_STOP_R 										PEout(10)														//PE10->右电机启停控制,		0启动,1停止

//边刷电机启停常量
#define M_SIDE_Run												1
#define M_SIDE_Stop												0

//边刷电机控制端口宏_输出
#define M_SIDE_L 													PEout(5)														//PE5
#define M_SIDE_R 													PEout(11)														//PE11

//风机启停常量
#define M_FAN_Run													1
#define M_FAN_Stop												0

//风机速度常量
#define M_FAN_speed_h											99
#define M_FAN_speed_m											60
#define M_FAN_speed_l											20
#define M_FAN_speed_stop									0

//LED显示屏控制端口宏_输出
#define	LED_DIN 													PDout(8)														//PD8
#define	LED_CLK 													PDout(9)														//PD9
#define	LED_STB 													PDout(10)														//PD10

//LED显示对应显示数组中的位置,NUM1-NUM4为从右到左的数码管
#define LED_NUM1													3
#define LED_NUM2													2
#define LED_NUM3													1
#define LED_NUM4													7
#define LED_SPOT													4
#define LED_PLAN													5
#define LED_CLEAN													6
#define LED_GARBAGE_NORMAL								8
#define LED_GARBAGE_WARNING								9
#define LED_HOME													10
#define LED_ERR_Bat_Low										11																	//显示电池电量低错误代码

//LED_电容按键下的LED灯显示代码
#define CODE_SPACE												11																	//LED不亮
#define CODE_SPOT  												0xE0																//电容按键SPOT下的LED灯亮
#define CODE_PLAN 												0x0E																//电容按键PLAN下的LED灯亮
#define CODE_CLEAN  											0x1E																//电容按键CLEAN下的LED灯亮
#define CODE_GARBAGE_NORMAL	 							0x0E																//电容按键垃圾桶下的LED灯亮_外圈_蓝色
#define CODE_GARBAGE_WARNING  						0xE0																//电容按键垃圾桶下的LED灯亮_内圈_红色
#define CODE_HOME  												0xF0																//电容按键HOME下的LED灯亮
#define CODE_E 														10																	//字母"E",代入LED_DISP函数从数组中调用code[10]=0x9E
#define CODE_L														0x1C																//字母"L"
#define CODE_o														0x3A																//字母"o"

//语音芯片控制端口宏_输出
#define	VOICE_RST PEout(3)																										//VOICE_RST->PE.3
#define VOICE_DIN	PEout(4)																										//VOICE_DIN->PE.4

//语音对应地址码
#define VOICE_Play_charging_up_en					0x00																//英文->"Charing up"
#define VOICE_Play_charging_start_en			0x01																//英文->"Charing start"
#define VOICE_Play_set_sure								0x02																//英文->"Set sure"????此处不确定,听不清
#define VOICE_Play_ding										0x03																//音效->"叮"
#define VOICE_Play_dong										0x04																//音效->"咚"
#define VOICE_Play_charging_up_ch					0x05																//中文->"电量已充满"
#define VOICE_Play_remind_long						0x06																//音效->长提示音
#define VOICE_Play_charging_start_ch			0x07																//中文->"开始充电"
#define VOICE_Play_remind_short						0x08																//音效->短提示音
#define VOICE_Play_set_success						0x0A																//中文->"设置成功"

//电容按键控制端口宏_输入
#define KEY0 															PBin(5)  														//读取按键0_SPOT
#define KEY1 															PBin(6)															//读取按键1_CLEAN
#define KEY2 															PBin(7)															//读取按键2_PLAN
#define KEY3 															PBin(8)															//读取按键3_HOME

//电容按键相关参数
#define KEY_SPOT													1
#define KEY_CLEAN													2
#define KEY_PLAN													3
#define KEY_HOME													4

//计划任务相关参数
#define PLAN_ALARM_CNT_VALUE							86400

//充电状态相关常量
#define SYS_Battery_Temp_High							2000																//电池温度高报警阈值
#define SYS_Battery_Low										1750																//电池电量低报警阈值,低于此值后机器人切换至自动充电状态
#define SYS_Battery_Very_Low							1650																//电池电量极低报警阈值,低于此值后为保护电池停止自动返回功能,仅报警提示手动充电
#define SYS_Power24V_Connected_Value			2000																//24V电源接入判断阈值
#define SYS_Power24V_Switch_err_Value			1500																//电源开关未打开判断阈值,当24V电源接入时,若此开关未打开,PC5上读取的电池电量ADC值接近0
#define SYS_Power24V_Switch_off_err_Value	2600																//充电过程中电源开关关闭判断阈值,当电压高于此值后,证明电源开关关闭
#define SYS_Battery_Full									2150																//电池充满电阈值，						 14.4V*1.15 	= 16.56V  Value_adc = (16.56*56/(56+470))*4096/3.3 = 2188.3
#define SYS_Battery_1d4										1850																//电池电量达到1/4满电判断阈值, 14.4V*0.85 	= 12.24V  Value_adc = (12.24*56/(56+470))*4096/3.3 = 12.24*132.14425 = 1617
#define SYS_Battery_2d4										1950																//电池电量达到2/4满电判断阈值, 14.4V*0.925 	= 13.32V  Value_adc = (13.32*56/(56+470))*4096/3.3 = 13.32*132.14425 = 1760
#define SYS_Battery_3d4										2050																//电池电量达到3/4满电判断阈值, 14.4V								  Value_adc = 14.4*132.14425 = 1903

//避障相关常量
#define SENS_F_L_EN_Value									2500																//前避障左传感器使能阈值
#define SENS_F_LM_EN_Value								3200																//前避障左中传感器使能阈值
#define SENS_F_M_EN_Value									3400																//前避障中传感器使能阈值
#define SENS_F_RM_EN_Value								3300																//前避障右中传感器使能阈值
#define SENS_F_R_EN_Value									3700																//前避障右传感器使能阈值

#define SENS_F_L_nEN											(sensor_val[0]>SENS_F_L_EN_Value)
#define SENS_F_LM_nEN											(sensor_val[1]>SENS_F_LM_EN_Value)
#define SENS_F_M_nEN											(sensor_val[2]>SENS_F_M_EN_Value)
#define SENS_F_RM_nEN											(sensor_val[3]>SENS_F_RM_EN_Value)
#define SENS_F_R_nEN											(sensor_val[4]>SENS_F_R_EN_Value)

//碰撞相关常量
#define SENS_CRUSH_EN											0

#define SENS_CRUSH_L_nEN									(sensor_val[5] 	!= SENS_CRUSH_EN)
#define SENS_CRUSH_R_nEN									(sensor_val[6] 	!= SENS_CRUSH_EN)

//探底相关常量,测试时SENS_B_EN为0，实际使用设为1
#define SENS_B_EN													1

#define SENS_B_L_nEN											(sensor_val[7] 	!= SENS_B_EN)
#define SENS_B_LM_nEN											(sensor_val[8] 	!= SENS_B_EN)
#define SENS_B_RM_nEN											(sensor_val[9] 	!= SENS_B_EN)
#define SENS_B_R_nEN											(sensor_val[10] != SENS_B_EN)

//传感器相关常量
#define SENS_F_L													0
#define SENS_F_L_ADC_CH										6
#define SENS_F_L_Value 										PAin(6)  														//PA6		1		前避障左

#define SENS_F_LM													1
#define SENS_F_LM_ADC_CH									3
#define SENS_F_LM_Value 									PAin(3)  														//PA3		2		前避障左中

#define SENS_F_M													2
#define SENS_F_M_ADC_CH										4
#define SENS_F_M_Value 										PAin(4)  														//PA4		3		前避障中

#define SENS_F_RM													3
#define SENS_F_RM_ADC_CH									7
#define SENS_F_RM_Value 									PAin(7)  														//PA7		4		前避障右中

#define SENS_F_R													4
#define SENS_F_R_ADC_CH										5
#define SENS_F_R_Value 										PAin(5)  														//PA5		5		前避障右

#define SENS_FI_L													5
#define SENS_FI_L_Value 									PBin(14)  													//PB14	6		前避障碰撞左
#define SENS_FI_R													6
#define SENS_FI_R_Value 									PBin(15)  													//PB15	7		前避障碰撞右

#define SENS_B_L													7
#define SENS_B_L_Value  									PEin(12)  													//PE12	1		探底检测左
#define SENS_B_LM													8
#define SENS_B_LM_Value 									PEin(13)  													//PE13	2		探底检测中左
#define SENS_B_RM													9
#define SENS_B_RM_Value 									PEin(15)														//PE15	3		探底检测中右
#define SENS_B_R													10
#define SENS_B_R_Value  									PEin(14)														//PE14	4		探底检测右

#define SENS_WHEEL_UP											11
#define SENS_WHEEL_UP_Value 							PAin(1)															//PA1		1		双轮抬起
#define SENS_NO_GARBAGE_BOX								12
#define SENS_NO_GARBAGE_BOX_Value					PCin(12)														//PC12	2		垃圾盒未放检测
#define SENS_DUSH_FULL										13
#define SENS_DUSH_FULL_Value							PCin(10)														//PC10	3		灰尘过多检测

#define SENS_CHARGE_BOX_CON								14
#define SENS_CHARGE_BOX_CON_Value 				PAin(2)															//PA2		1		充电座连接检测
#define SENS_CB_LOC_L											15
#define SENS_CB_LOC_L_Value 							PDin(15)														//PD15	2		充电座定位接收左
#define SENS_CB_LOC_LM										16
#define SENS_CB_LOC_LM_Value 							PDin(13)														//PD13	3		充电座定位中左
#define SENS_CB_LOC_RM										17
#define SENS_CB_LOC_RM_Value 							PDin(14)														//PD14	4		充电座定位中右
#define SENS_CB_LOC_R											18
#define SENS_CB_LOC_R_Value								PDin(12)														//PD12	5		充电座定位接收右

//正常为0,过载为1
#define SENS_FAN_OVLOAD										19
#define SENS_FAN_OVLOAD_Value							PDin(3)															//PD3		1		吸尘风机过载检测
#define SENS_M_SIDE_L_OVLOAD							20
#define SENS_M_SIDE_L_OVLOAD_Value				PDin(4)															//PD4		2		左边刷电机过载检测
#define SENS_M_SIDE_R_OVLOAD							21
#define SENS_M_SIDE_R_OVLOAD_Value				PDin(5)															//PD5		3		右边刷电机过载检测
#define SENS_M_MOVE_L_OVLOAD							22
#define SENS_M_MOVE_L_OVLOAD_Value				PDin(6)															//PD6		4		左行走电机过载检测
#define SENS_M_MOVE_R_OVLOAD							23
#define SENS_M_MOVE_R_OVLOAD_Value				PDin(7)															//PD7		5		右行走电机过载检测

#define SENS_Battery											24
#define SENS_Battery_ADC_CH								15
#define SENS_Battery_Value								PCin(5)															//PC5		1		电池电量检测

#define SENS_24V													25
#define SENS_Charging_Battery_ADC_CH			14
#define SENS_Charging_Battery_Value				PCin(4)															//PC4		2		充电时电源电量检测

#define SENS_Battery_temp									26
#define SENS_Battery_temp_ADC_CH					10
#define SENS_Battery_temp_Value						PCin(0)															//PC0		3		电池温度检测

//全局变量声明
extern u8 g_charge_led_horse_lamp_cnt;
extern u8 g_charge_start_flag;
extern const u16 m_move_length_r;
extern const u16 m_move_length_l;
extern const u16 m_move_encoder_num;
extern u8 led_code[];
extern u8	g_RTC_1s_charging_led_flag;
