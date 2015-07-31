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

//�����˻���״̬����
static u8	ROB_Basic_Status_flag = 0;																					//�����˻���״̬��ʶ,1->����,2->��ɨ,3->�Զ����س����,4->���,5->����,6->����

//��������״̬��ز���
typedef union{
	u16 ROB_Sub_Status;
	struct{
		u8 sys_err_flag:1;																														//bit0	,ϵͳ���쳣��ʶ
		u8 battery_Low_flag:1;																												//bit1	,��ص����ͱ�ʶ
		u8 battery_very_low_flag:1;																										//bit2	,��ص������ͱ�ʶ
		u8 battery_temp_high_flag:1;																									//bit3	,����¶ȸ߱�ʶ
		u8 motor_overload_flag:1;																											//bit4	,������ر�ʶ
		u8 wheel_up_err_flag:1;																												//bit5	,˫��̧���ʶ
		u8 garbage_box_err_flag:1;																										//bit6	,����δ�ű�ʶ
		u8 garbage_box_full_flag:1;																										//bit7	,��������ʶ
		u8 power_connected_flag:1;																										//bit8	,��ͨ24V����Դ��ʶ
		u8 charge_alignment_en_flag:1;																								//bit9	,��ʼ�������Խӱ�ʶ
		u8 charge_start_flag:1;																												//bit10	,��翪ʼ��ʶ
		u8 battery_full_flag:1;																												//bit11	,�����ѳ�����ʶ
		u8 charge_finished_flag:1;																										//bit12	,�����ɱ�ʶ
		u8 plan_en_flag:1;																														//bit13	,��ʼ���üƻ������ʶ
		u8 plan_cnt_start_flag:1;																											//bit14	,�ƻ����񵹼�ʱ��ʼ��ʶ
		u8 plan_time_up_flag:1;																												//bit15	,�ƻ�����ʱ�䵽��ʶ
	}ROB_Sub_Status_bit;
}ROB_Sub_Status_Union_Def;
ROB_Sub_Status_Union_Def ROB_Sub_Status_Union;

//��������ز���
static u16 sensor_val[27];																										//�洢�������ɼ����ݵ�����

//������->������ز���
static union{
	u8 Sens_Avoid_Front_Value;
	struct{
		u8 sens_avoid_front_r_value:1;																								//bit0	,ǰ������
		u8 sens_avoid_front_rm_value:1;																								//bit1	,ǰ��������
		u8 sens_avoid_front_m_value:1;																								//bit2	,ǰ������
		u8 sens_avoid_front_lm_value:1;																								//bit3	,ǰ��������
		u8 sens_avoid_front_l_value:1;																								//bit4	,ǰ������
	}Sens_Avoid_Front_Value_bit;
}Sens_Avoid_Front_Value_Union;

//������->ǰ��ײ��ز���
static union{
	u8 Sens_Avoid_Crush_Value;
	struct{
		u8 sens_avoid_crash_l_value:1;																								//bit0	,ǰ��ײ��
		u8 sens_avoid_crash_r_value:1;																								//bit1	,ǰ��ײ��
	}Sens_Avoid_Crush_Value_bit;
}Sens_Avoid_Crush_Value_Union;

//������->̽����ز���
static union{
	u8 Sens_Avoid_Bottom_Value;
	struct{
		u8 sens_avoid_bottom_l_value:1;																								//bit0	,̽����
		u8 sens_avoid_bottom_lm_value:1;																							//bit1	,̽������
		u8 sens_avoid_bottom_rm_value:1;																							//bit2	,̽������
		u8 sens_avoid_bottom_r_value:1;																								//bit3	,̽����
	}Sens_Avoid_Bottom_Value_bit;
}Sens_Avoid_Bottom_Value_Union;

////������->�������λ��ز���
//static union{
//	u8 Sens_Avoid_Charge_Box_Location_Value;
//	struct{
//		u8 sens_charge_box_location_l_value:1;																				//bit0	,�������λ��
//		u8 sens_charge_box_location_lm_value:1;																				//bit1	,�������λ����
//		u8 sens_charge_box_location_rm_value:1;																				//bit2	,�������λ����
//		u8 sens_charge_box_location_r_value:1;																				//bit3	,�������λ��
//	}Sens_Avoid_Charge_Box_Location_Value_bit;
//}Sens_Avoid_Charge_Box_Location_Value_Union;

//LED��ʾ������,ȫ�ֱ���
u8 led_code[]		={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};										//����LED��ʾ�����Ե�����,��һ�ֽ���Ч

//���ݰ�����ز���
static u8 key_keysta = 0;																											//��ŵ�ǰ��ֵ
static u8 key_exti_enter_flag = 0;																						//���밴���жϱ�ʶ,�˳��жϺ��ڰ���������������

//���ߵ������
static u8 m_move_speed 	= 25;																									//���ߵ���ٶ�,��������TIM8_CH2&TIM8_CH3 COMPAREֵ�Ե���PWMռ�ձ�,20%
//static u8 sens_ir_led_brightness = 42;																				//���Ϸ���LEDǿ��ֵ,ռ�ձ�42%
static u8 m_fan_speed		= 30;																									//����ٶ�,��������TIM8_CH1 COMPAREֵ�Ե���PWMռ�ձ�,30%

//���ߵ�����������,ȫ�ֱ���
const u16 m_move_length_r     		= 30;																				//�����ְ뾶
const u16 m_move_length_l     		= 150;																			//���������ĵ����������ĵľ���
const u16 m_move_encoder_num			= 900;																			//������תһȦ��Ӧ���̼���ֵ

//���ߵ��_��������ת�Ƕȳ���
//static const u16 m_move_ledge_l_angle 	= 30;																	//�����������ϴ������н�
//static const u16 m_move_l_lm_angle 			= 30;																	//����ϴ����������б��ϴ������н�
//static const u16 m_move_lm_m_angle 			= 30;																	//���б��ϴ��������б��ϴ������н�
//static const u16 m_move_m_rm_angle 			= 30;																	//�б��ϴ����������б��ϴ������н�
//static const u16 m_move_rm_r_angle			= 30;																	//���б��ϴ��������ұ��ϴ������н�
//static const u16 m_move_r_redge_angle 	= 90;																	//�ұ��ϴ��������ұ��ּн�
//static const u16 m_move_redge_m_angle 	= 90;																	//�ұ������б��ϴ������н�
static const u16 m_move_ledge_m_angle 	= 90;																	//��������б��ϴ������н�
static const u16 m_move_backward_angle 	= 90;																	//�����˺����õĽǶ�ֵ,�ɻ���ɾ���
static const u16 m_move_turn_around_angle = 180;															//�����˵�ͷ�ýǶ�ֵ
static u16 m_move_usr_angle = 30;																							//���ߵ��ת��Ƕȱ���

//������תm_move_usr_angle�Ƕ�,����Ӧ��������ֵ:encoder_target_num = (usr_angle*length_l*encoder_num)/(360*length_r)

//����оƬ��ز���
static u8	voice_play_times_charging_start_ch = 1;															//"��ʼ���"�������Ŵ���,��ʼ��Ϊ���Ŵ���ֵ,ʹ��ʱ�ݼ�ֱ��Ϊ0,Ϊ0���������ٲ���
static u8	voice_play_times_charging_finished_ch = 1;													//"�����ѳ���"�������Ŵ���

//�����ز���,ȫ�ֱ���
u8 g_charge_led_horse_lamp_cnt = 0;
u8 g_charge_start_flag = 0;

//���PWM��ز���
static u8 charge_pwm_pulse_width = 0;

//TIM5��ʱ����
static u8 timer5_500ms_flag = 0;																							//��ʱ����ʱ��500ms��ʶ
static u8 timer5_1s_charging_led_flag	= 0;																		//��ʱ����ʱ��1s��ʶ,���ڳ��ʱLED��ˮ����˸��ʱ
static u8 timer5_1s_switch_err_flag = 0;																			//��ʱ����ʱ��1s��ʶ,���ڳ��ʱ��Դ����δ��������ʾ����ʱ
static u8 timer5_1s_charging_1d4_flag = 0;																		//��ʱ����ʱ��1s��ʶ,���ڳ��ʱ��ص�������1/4�������ʱ�ĳ��ռ�ձȱ仯��ʱ
//static u8 timer5_1s_motor_fan_flag = 0;																				//��ʱ����ʱ��1s��ʶ,���ڷ����������ʱ
static u8 timer5_1s_Dush_Full_flag = 0;																				//��ʱ����ʱ��1s��ʶ,������������ʾ����ʱ
static u8 timer5_1s_battery_very_low_flag = 0;																//��ʱ����ʱ��1s��ʶ,��ص������ʹ�����ʾ����ʱ
static u8 timer5_1min_flag = 0;																								//��ʱ��1���ӱ�ʶ
//static u8 timer5_1hour_flag = 0;																							//��ʱ��1Сʱ��ʶ,���ڼƻ�����Сʱ����ʱ
static u8 timer5_24hour_flag = 0;																							//��ʱ��24Сʱ��ʶ,���ڼƻ�����ÿ����ͬʱ���ظ�����ʱ
static u8 timer5_cnt_10ms = 0;
static u8 timer5_cnt_100ms = 0;
static u8 timer5_cnt_500ms = 0;
static u8 timer5_cnt_1s = 0;
static u8 timer5_cnt_1min = 0;
static u8 timer5_cnt_30min = 0;
static u8 timer5_cnt_1hour = 0;
static u8 timer5_cnt_24hour = 0;

//�ƻ�������ز���
static u8 plan_usr_min_value 	= 0;																						//�û����õļƻ����񵹼�ʱʱ��->����
static u8 plan_usr_hour_value 	= 0;																					//�û����õļƻ����񵹼�ʱʱ��->Сʱ
static u8 plan_cnt_min_value = 0;																							//�ƻ����񵹼�ʱʱ��->����
static u8 plan_cnt_hour_value = 0;																						//�ƻ����񵹼�ʱʱ��->Сʱ
static u8 plan_step_change_flag = 0;																					//�ƻ��������ò���ֵ�л���ʶ,1��Ӧ���ò���ֵΪ10,0��Ӧ���ò���ֵΪ1;
//static u8 plan_24hour_time_up_flag = 0;																				//24Сʱ����ʱʱ�䵽��ʶ

//ϵͳ��ʼ��
void Usr_System_Init(void)
{
	delay_init();	    	 																													//��ʱ������ʼ��
	NVIC_Configuration(); 	 																											//����NVIC�жϷ���2:2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
	UART_init(9600);	 																														//���ڳ�ʼ��,������9600
	TIM5_Int_Init(999,719);																												//TIM5��ʼ��,����10ms��ʱ��׼
	LED_Init();																																		//LED��ʾ����ʼ��
	KEY_Init();																																		//���ݰ�����ʼ��
	REMOTE_Init();																																//������ն˿ڳ�ʼ��
	VOICE_Init();																																	//����оƬ�˿ڳ�ʼ��
	POWER5V_Init();																																//5V��Դ���ƶ˿ڳ�ʼ��
	SENSOR_Init();																																//�������˿ڳ�ʼ��
 	MOTOR_SIDE_Init();		  																											//��ˢ����˿ڳ�ʼ��
  MOTOR_FAN_Init();          																										//��������˿ڳ�ʼ��
	MOTOR_MOVE_Init();																														//���ߵ���˿ڳ�ʼ��
	TIM_CtrlPWMOutputs(TIM8, ENABLE);																							//ʹ��TIM8_PWM������,ʹ�����ߵ��,ʹ�ܷ��,ʹ�ܱ��Ϸ���
	CHARGER_Init();																																//�����ƶ˿ڳ�ʼ��

//	VOICE_PLAY(VOICE_Play_remind_long);																						//��ʼ�������ʾ��
	printf("System_Init finished!\r\n");																					//��ʼ���ɹ�
}

//����LED��ʾ������ǰ���Ƚ�LED��ʾ��������,��ֹ��������ʱ����֮ǰ����ʾ
void Disp_Loop(void)
{
	u8 i;

	for(i=0;i<8;i++)
		led_code[i] = 0x00;

	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){											//�����üƻ������ʶ��λ,����ʾ����ʱ����ʱ��
		LED_DISP_Time(led_code, plan_usr_hour_value, plan_usr_min_value);
	}
}

//���ݰ����жϴ�����
void EXTI9_5_IRQHandler(void)
{
	key_exti_enter_flag = 1;
	if(EXTI_GetFlagStatus(EXTI_Line5) != RESET){
		key_keysta = KEY_SPOT;																													//SPOT����
		ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop,M_FAN_speed_stop);
	}else if(EXTI_GetFlagStatus(EXTI_Line6) != RESET){
		key_keysta = KEY_CLEAN;																													//CLEAN����
	}else if(EXTI_GetFlagStatus(EXTI_Line7) != RESET){
		key_keysta = KEY_PLAN;																													//PLAN����
	}else if(EXTI_GetFlagStatus(EXTI_Line8) != RESET){
		key_keysta = KEY_HOME;																													//HOME����
	}else
		key_keysta = 0;
	EXTI->PR  |= (1<<5)|(1<<6)|(1<<7)|(1<<8);																			//���LINE5,6,7,8�ϵ��жϱ�ʶ
}

//���ݰ�����ֵ������
void Key_Loop(void)
{
	if(key_exti_enter_flag){
		switch(key_keysta){
			case(KEY_SPOT):																														//SPOT������
				LED_DISP(led_code, LED_SPOT, 1);																							//SPOT������
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){																			//��������״̬ʱ,SPOT����Ч

				}else if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){																		//������ƻ���������״̬ʱ,SPOT�����������Ӽƻ�����ʱ��,����1min
					plan_step_change_flag = 0;																								//����ֵ����Ϊ1
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
			case(KEY_PLAN):																														//PLAN������
				LED_DISP(led_code, LED_PLAN, 1);																							//PLAN������
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){
					if(plan_usr_hour_value||plan_usr_min_value){
						plan_cnt_min_value 	= 0;																									//���û����õ���ʱʱ����ֵ,��ȷ������ʱ,����ʱ����������
						plan_cnt_hour_value = 0;																									//���û����õ���ʱʱ����ֵ,��ȷ������ʱ,����ʱ����������
						timer5_24hour_flag = 0;																										//���û����õ���ʱʱ����ֵ,��ȷ������ʱ,24Сʱ��ʱ����ʶ����
						timer5_cnt_1min = 0;																											//���û����õ���ʱʱ����ֵ,��ȷ������ʱ,1���ӵ���ʱ����ֵ����
						timer5_cnt_1hour = 0;																											//���û����õ���ʱʱ����ֵ,��ȷ������ʱ,1Сʱ����ʱ����ֵ����
						timer5_cnt_24hour = 0;																										//���û����õ���ʱʱ����ֵ,��ȷ������ʱ,24Сʱ����ʱ����ֵ����
						ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag = 1;					//���û����õ���ʱʱ����ֵ,��ȷ������ʱ,�ƻ����񵹼�ʱ��ʼ��ʶ��λ
						VOICE_PLAY(VOICE_Play_set_success);																				//���û����õ���ʱʱ����ֵʱ,��ʾ���óɹ�
					}
					ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag = 0;									//���üƻ�����ʹ�ܱ�ʶ����
				}else{
					ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag = 1;									//���üƻ�����ʹ�ܱ�ʶ��λ
				}
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;														//�л�������״̬
				break;
			case(KEY_CLEAN):																													//CLEAN������
				LED_DISP(led_code, LED_CLEAN,	1);																					//CLEAN������
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){									//������ƻ���������״̬ʱ,CLEAN�����ڼ��ټƻ�����ʱ��,����1min
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
					ROB_Basic_Status_flag = ROB_Basic_Cleaning_flag;													//�л�����ɨ״̬
				}
				break;
			case(KEY_HOME):																														//HOME������
				LED_DISP(led_code, LED_HOME, 1);																					//HOME������
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_en_flag){
					plan_step_change_flag = 1;																								//����ֵ����Ϊ10
					if(plan_usr_min_value<60){
						plan_usr_min_value += 10;
					}else{
						plan_usr_hour_value += 1;
						plan_usr_min_value = 0;
					}
				}else{

				}
				//ROB_Basic_Status_flag = ROB_Basic_Returning_flag;														//�л����Զ����״̬
				break;
			default:

				break;
		}
		key_exti_enter_flag = 0;
	}
}

//����ң�ؽ��մ�����
void Remote_loop(void)
{
	u8 Rmtkey = 0;																																//����ң�ؽ��յ��ļ�ֵ

	Rmtkey=REMOTE_Scan();																													//����ң�ؽ��ռ�ֵɨ��

	if(Rmtkey){
		switch(Rmtkey){
			case 0:																																				//str="ERROR";

				break;
			case 162:																																			//str="POWER";
				if(ROB_Basic_Status_flag == ROB_Basic_Cleaning_flag)
					ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//���ڴ�ɨ״̬,�л�������״̬
				else
					ROB_Basic_Status_flag = ROB_Basic_Cleaning_flag;															//�����ڴ�ɨ״̬,�л�����ɨ״̬
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
				TIM_SetCompare2(TIM8,m_move_speed);																						//���������ߵ��ת��
				TIM_SetCompare3(TIM8,m_move_speed);																						//���������ߵ��ת��
//				TIM_SetCompare4(TIM8,sens_ir_led_brightness);																					//���ñ��Ϸ���LED����ǿ��
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
				TIM_SetCompare2(TIM8,m_move_speed);																						//���������ߵ��ת��
				TIM_SetCompare3(TIM8,m_move_speed);																						//���������ߵ��ת��
//			TIM_SetCompare4(TIM8,sens_ir_led_brightness);																						//���ñ��Ϸ���LED����ǿ��
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

//������״̬��ȡ����
void Sensor_Loop(void)
{
	u8 i;

	for(i=0;i<=6;i++)			sensor_val[i] = SENSOR_Scan(i);													//sensor[0..6]ǰ���ϴ�������ǰ������ײ���������ݶ�ȡ

	for(i=7;i<=10;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[7..10]̽�״��������ݶ�ȡ

	for(i=11;i<=13;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[11..13]����״̬���������ݶ�ȡ(˫��̧��,������δ�ż��,�ҳ�������)

	//for(i=14;i<=18;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[14..18]������Գ����ش��������ݶ�ȡ,����ֱ�Ӷ�IO��ƽ,��Ҫ��remote.c�д���

	for(i=19;i<=23;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[19..23]���ش��������ݶ�ȡ

	for(i=24;i<=26;i++)		sensor_val[i] = SENSOR_Scan(i);													//sensor[24..26]��ص������,���ʱ��Դ�������,����¶ȼ��

	//printf("ROB_Sub_Status=%d\r\n",ROB_Sub_Status_Union.ROB_Sub_Status);					//��ӡϵͳ״̬�Ĵ���ֵ
	if(timer5_500ms_flag){
		for(i=0;i<=10;i++)		printf("[%d]=%d,",i,sensor_val[i]);											//����������ֵ��ӡ,������
		printf("\r\n");
		timer5_500ms_flag = 0;
	}

	//�����ϵ�����������ȡֵ��ֵ�������������壬����������ʹ��
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

//���ϼ��Զ����ص�������
void Navigation_Loop(u8 nav_avoid_en_flag, u8 nav_return_en_flag)
{
	if(nav_avoid_en_flag){
		//̽�״��������ȼ����,4������������ϳ�16��״̬,����ʱ�ж�״̬��ʵ���෴�������"usr_include.h"�еĺ궨��
		switch(Sens_Avoid_Bottom_Value_Union.Sens_Avoid_Bottom_Value){
			case(0):	//			printf("̽�״�����L,LM,RM,Rͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(1):	//			printf("̽�״�����L,LM,RMͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(2):	//			printf("̽�״�����L,RM,Rͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(3):	//			printf("̽�״�����RM,Rͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(4):	//			printf("̽�״�����L,LM,Rͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(5):	//			printf("̽�״�����LM,Rͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(6):	//			printf("̽�״�����L,Rͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(7):	//			printf("̽�״�����R����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(8):	//			printf("̽�״�����L,LM,RMͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(9):	//			printf("̽�״�����LM,RMͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(10):	//			printf("̽�״�����L,RMͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(11):	//			printf("̽�״�����L,LM,Rͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(12):	//			printf("̽�״�����L,LMͬʱ����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(13):	//			printf("̽�״�����LM����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(14):	//			printf("̽�״�����L����!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			default:	//			printf("̽�״�����δ����!\r\n");

				break;
		}

		//̽�״�����δ����,��ײ���������ȼ���,2������������ϳ�4��״̬,��ײ�������ж����ݵ�����ʵ��һ��
		switch(Sens_Avoid_Crush_Value_Union.Sens_Avoid_Crush_Value){
			case(0):	//				printf("ǰ����ײ�ϰ���!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(1):	//				printf("����ײ�ϰ���!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(2):	//				printf("�ҷ���ײ�ϰ���!\r\n");
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			default:	//				printf("δ������ײ!\r\n");

				break;
		}

		//��ײ������δ����,ʹ�ñ��ϴ��������е���,5������������ϳ�32��״̬
		switch(Sens_Avoid_Front_Value_Union.Sens_Avoid_Front_Value){
			case(0):	//				printf("ǰ�����������ǽ��!\r\n");															0x00->00000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(1):	//				printf("��ǰ����ǽ��,��෽��ͨ��!");														0x01->00001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_AntiClkWiz, m_move_speed);
				break;
			case(2):	//				printf("��ǰ����ǽ��,��෽���ϰ���!\r\n");											0x02->00010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(3):	//				printf("��ǰ����ǽ��,��෽��ͨ��!\r\n");												0x03->00011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(4):	//				printf("ǰ������������ϰ���!\r\n");														0x04->00100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(5):	//				printf("�ҷ����ϰ���,����п��ƹ��ϰ���!\r\n");									0x05->00101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(6):	//				printf("�ҷ����ϰ���,����в���ͨ����ͨ��!\r\n");								0x06->00110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(7):	//				printf("�Ҳ෽���ϰ���,��ǰ����ͨ��!\r\n");											0x07->00111
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(8):	//				printf("��ǰ���в���ͨ����ͨ��,��ǰ����ǽ��!\r\n");							0x08->01000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(9):	//				printf("��ǰ���в���ͨ����ͨ��,��෽����ͨ��!\r\n");						0x09->01001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(10):	//				printf("������������޷�ͨ����ͨ��!\r\n");											0x0A->01010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(11):	//				printf("��ǰ�����޷�ͨ����ͨ��,�󷽿���ͨ��!\r\n");							0x0B->01011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(12):	//				printf("��ǰ�����޷�ͨ����ͨ��,��෽���ϰ���!\r\n");						0x0C->01100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(13):	//				printf("��ǰ�����޷�ͨ����ͨ��,��෽����ͨ��!\r\n");						0x0D->01101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(14):	//				printf("ǰ�����޷�ͨ����ͨ��,�����������ϰ���!\r\n");						0x0E->01110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(15):	//				printf("�Ҳ෽���ϰ���,��ǰ������ͨ��,�������Ҳ෽ǽ��!\r\n");	0x0F->01111
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(16):	//				printf("��ǰ����ͨ��,��෽���ϰ���!\r\n");											0x10->10000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(17):	//				printf("ǰ����ǽ,����������ͨ��!\r\n");													0x11->10001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(18):	//				printf("�Ҳ෽��ͨ��,��ǰ�����޷�ͨ����ͨ��!\r\n");							0x12->10010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(19):	//				printf("�Ҳ෽��ͨ��,��෽�п���ͨ����ͨ��!\r\n");							0x13->10011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(20):	//				printf("��ǰ���п��Դ��Ҳ��ƹ����ϰ���,��෽���ϰ���!\r\n");		0x14->10100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(21):	//				printf("��ǰ������ǰ�����п����ƹ����ϰ���!\r\n");							0x15->10101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(22):	//				printf("��ǰ���п����ƹ����ϰ���,��ǰ�����޷�ͨ����ͨ��!\r\n");	0x16->10110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(23):	//				printf("��ǰ���п����ƹ����ϰ���,��෽��ͨ��!\r\n");						0x17->10111
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(24):	//				printf("�Ҳ෽��ͨ��,��ǰ����ǽ��!\r\n");												0x18->11000
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(25):	//				printf("�Ҳ෽��ͨ��(ͨ�������Դ�),��෽��ͨ��(ͨ��������С)!\r\n");	0x19->11001
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(26):	//				printf("ǰ���пɴ��Ҳ෽�ƹ����ϰ���,��ǰ�����޷�ͨ����ͨ��!\r\n");		0x20->11010
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(27):	//				printf("ǰ���п��ƹ����ϰ���,���Ҳ�ͨ���Ŀ�����һ��!\r\n");						0x21->11011
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(28):	//				printf("�Ҳ෽��ͨ��,��෽���ϰ���!\r\n");														0x22->11100
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(29):	//				printf("�Ҳ෽��ͨ��,��෽�п��ƹ����ϰ���!\r\n");										0x23->11101
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			case(30):	//				printf("��ǰ����ͨ��,��෽���ϰ���!\r\n");														0x24->11110
				MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_ledge_m_angle, M_MOVE_ClkWiz, m_move_speed);
				break;
			default:	//				printf("���ϰ�\r\n");																									0x25->11111
				MOTOR_MOVE(M_MOVE_Forward,m_move_speed);
				break;
		}
	}else{																																				//���ϵ�������
		if(nav_return_en_flag)																												//���Զ����ص���ʹ��,��ֹͣ���
			;
		else{																																					//������ȫ������,���ȫ��ֹͣ
			MOTOR_MOVE(M_MOVE_Forward,m_move_speed);
			MOTOR_SIDE(M_SIDE_Stop);
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		}
	}
	if(nav_return_en_flag){																												//���Զ����ر�ʶʹ��,�����Զ����ص���,���������Ѷ�׼���жԽ�ʱ,Ӧ���ܱ��Ϲ���
		//if(){
			//;
		//}else if(){
			//ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_alignment_en_flag = 0;					//����׼������ʧ���������׼��ʶ
		//}else{
			//ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_alignment_en_flag = 1;					//�����������������,�Զ��Խӿ�ʼ��ʶ��λ,�˱�־��λ��Ӧ���ܱ��Ϲ���
		//}
	}else
		;
}

//���ʱ����ָʾ�ƴ�����,ͨ��LED��˸״̬��ʾ������
void Charging_LED_Horse_Lamp(u8 led_horse_lamp_en)
{
	if(led_horse_lamp_en){																												//����翪ʼ��ʶ��λ,��ʼ��ʾ������
		if(sensor_val[SENS_Battery] < SYS_Battery_1d4){																//��ص���С��1/4����״̬ʱ,4�������LED������������"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(5);
				timer5_1s_charging_led_flag = 0;
			}
		}else if(sensor_val[SENS_Battery] < SYS_Battery_2d4){													//��ص���С��2/4����״̬ʱ,��1����ܳ���"1",����3�������LED������������"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(4);
				timer5_1s_charging_led_flag = 0;
			}
		}else if(sensor_val[SENS_Battery] < SYS_Battery_3d4){													//��ص���С��3/4����״̬ʱ,��1�͵�2����ܳ���"1",����2�������LED������������"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(3);
				timer5_1s_charging_led_flag = 0;
			}
		}else if(sensor_val[SENS_Battery] < SYS_Battery_Full){												//��ص���С������״̬ʱ,��1����2�͵�3����ܳ���"1",����1�������LED������������"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(2);
				timer5_1s_charging_led_flag = 0;
			}
		}else{																																				//��ص�������ʱ,4������ܳ���"1"
			if(timer5_1s_charging_led_flag){
				LED_Horse_Lamp(1);
				timer5_1s_charging_led_flag = 0;
			}else
				;
		}
	}
}

//���״̬������
void Charging_Loop(void)
{
	Charging_LED_Horse_Lamp(1);																										//����翪ʼ��ʶ��λ��,��ʼ������ˮ����ʾ
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){
		if(sensor_val[SENS_Battery] < SYS_Battery_Full){															//����δ����
			MOTOR_MOVE_STOP();																														//��翪ʼ��,���ߵ��ֹͣ
			MOTOR_SIDE(M_SIDE_Stop);																											//��翪ʼ��,��ˢ���ֹͣ
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);																			//��翪ʼ��,���ֹͣ
			if(sensor_val[SENS_Battery] < SYS_Battery_1d4){
				if(timer5_1s_charging_1d4_flag){
					CHARGER_Start_1d4(charge_pwm_pulse_width);																		//����С��1/4�������ʱ,ʹ��ռ�ձȽ�����
					timer5_1s_charging_1d4_flag = 0;
				}
			}else if(sensor_val[SENS_Battery] < SYS_Battery_3d4){
				CHARGER_Start_more_than_1d4();																								//��������1/4�������С��3/4�������ʱ,ʹ�ù̶�ռ�ձȳ��,ռ�ձ�45.5%
			}else{
				CHARGER_Startt_more_than_3d4();																								//��������3/4�������ʱ,ʹ�ù̶�ռ�ձȳ��,ռ�ձ�60%
			}
		}else{																																				//�����ѳ���
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_finished_flag = 1;							//�����ɱ�ʶ��λ
			CHARGER_Stop();																																//����ѹ���ڵ�������ֵ��,ֹͣ���
			if(voice_play_times_charging_finished_ch){
				VOICE_PLAY(VOICE_Play_charging_up_ch);																				//����������������ʾһ��
				voice_play_times_charging_finished_ch--;
			}else
				voice_play_times_charging_finished_ch = 0;
		}
	}else
		CHARGER_Stop();
}

//����״̬������
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

//�ƻ���������
void Plan_Loop(void)
{
	if(timer5_24hour_flag){																												//��24Сʱ��ʱʱ�䵽��ʶ��λ,��ʹ�ܵ���ʱ
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag = 1;
		timer5_24hour_flag = 0;
	}
	if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_cnt_start_flag){							//���ƻ����񵹼�ʱʹ��,��ʼ���ݶ�ʱ��ʶ���м���
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
	//�жϼ�ʱʱ���Ƿ���ƻ�����ʱ����ͬ,��ͬ����λ��ʱʱ�䵽��ʶ
	if(plan_usr_hour_value == 0){
		if(plan_usr_min_value == 0){																									//���û����üƻ����񵹼�ʱʱ�䶼Ϊ0ʱ,�����е���ʱ�Ƚ�
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag = 0;
		}else{																																				//���û����üƻ����񵹼�ʱʱ���еķ���ֵ��Ϊ0ʱ,���е���ʱ�Ƚ�
			if(plan_cnt_min_value == plan_usr_min_value){
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag = 1;							//������ʱ���������û����÷�����һ��ʱ,ʱ�䵽��ʶ��λ
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

//��״̬������
//���ݴ�����״̬����Ӧ��״̬��ʶ������λ������,����ǰ���״̬������⵽
void Sub_Status_Loop(void)
{
	//�����ʱ��ص������,�������"Lo"
	if(ROB_Basic_Status_flag != ROB_Basic_Charging_flag){
		if(sensor_val[SENS_Battery] < SYS_Battery_Low){
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag = 1;
			LED_DISP_ERR(led_code, 1,8);																									//�����ͺ���ʾ�������"E18"
			MOTOR_SIDE(M_SIDE_Stop);																											//�����ͺ�ֹͣʹ�ñ�ˢ
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);																			//�����ͺ�ֹͣʹ�÷��
			if(sensor_val[SENS_Battery] < SYS_Battery_Very_Low){													//��ص������ڵ���������ֵ��Ķ���
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag = 1;
				LED_DISP(led_code,LED_ERR_Bat_Low, 1);																				//LED����ʾ��ص�������ʾ,"Lo"
				if(timer5_1s_battery_very_low_flag){
					VOICE_PLAY(VOICE_Play_dong);																									//��ص�������ʱͨ����ʾ������
					timer5_1s_battery_very_low_flag = 0;
				}
			}else{
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag = 0;
			}
		}else{
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag = 0;
		}
	}

	//�����,��24V���뵫δ������Դ����,�������"E10"
	if(sensor_val[SENS_24V] > SYS_Power24V_Connected_Value){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.power_connected_flag = 1;
		if(sensor_val[SENS_Battery] < SYS_Power24V_Switch_err_Value){									//��24V��Դ��ͨʱ,����ص�����ⲻ��ʱ,���ж��ǵ�Դ����δ��
			LED_DISP_ERR(led_code, 1, 0);																									//LED����ʾ�������,"E10"
			CHARGER_Stop();
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 0;
			if(timer5_1s_switch_err_flag){
				VOICE_PLAY(VOICE_Play_dong);
				timer5_1s_switch_err_flag = 0;
			}else
				;
		}else if(sensor_val[SENS_Battery] > SYS_Power24V_Switch_off_err_Value){				//����翪ʼ��,�������;�رյ�Դ����,ֹͣ����ٴα���
			LED_DISP_ERR(led_code, 1, 0);																									//LED����ʾ�������,"E10"
			CHARGER_Stop();
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 0;
		}else{
			ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 1;								//��ʼ����ʶ��λ
			if(voice_play_times_charging_start_ch){																				//��ʼ���������ʾ,����(VOICE_PALY_Times_charging_start_ch)��
				VOICE_PLAY(VOICE_Play_charging_start_ch);
				voice_play_times_charging_start_ch --;
			}else
				voice_play_times_charging_start_ch = 0;
		}
	}else{
		CHARGER_Stop();																																//û�н���24V��Դʱ�رճ��ͨ·
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.power_connected_flag = 0;
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag = 0;
	}
	g_charge_start_flag = ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag;//���ڽ���ʶ���ݸ�sensor.c

	//˫��̧����,�������"E11"(��ѯ��ʽ,���˫��̧���ⲿ�жϷ������ʹ��)
	if(sensor_val[SENS_WHEEL_UP] == 0){
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag = 1;								//˫��̧���ʶ��λ
		LED_DISP_ERR(led_code, 1,1);																									//˫��̧�����,��ʾ�������"E11"
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag = 0;
	}

	//���ر������,�������"E12"
	if((sensor_val[SENS_FAN_OVLOAD])||(sensor_val[SENS_M_SIDE_L_OVLOAD])||(sensor_val[SENS_M_SIDE_R_OVLOAD])||(sensor_val[SENS_M_MOVE_L_OVLOAD])||(sensor_val[SENS_M_MOVE_R_OVLOAD])){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag = 1;
		MOTOR_MOVE_AVOID(m_move_backward_angle, m_move_turn_around_angle, M_MOVE_ClkWiz, m_move_speed);
		LED_DISP_ERR(led_code, 1,2);																									//��ʾ�������"E12"
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag = 0;
	}

	//����¶ȸ�״̬���,�������"E13"
	if(sensor_val[SENS_Battery_temp] > SYS_Battery_Temp_High){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag = 1;
		LED_DISP_ERR(led_code, 1,3);																											//����¶ȸߴ�����ʾ,��ʾ"E13"
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		VOICE_PLAY(VOICE_Play_ding);																											//����¶ȸߴ�����ʱ,���Ͻ���������ʾ
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag = 0;
	}

	//����δ��״̬���,�������"E14"
	if(sensor_val[SENS_NO_GARBAGE_BOX] == 0){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag = 1;
		MOTOR_MOVE_STOP();
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		LED_DISP_ERR(led_code, 1,4);																											//����δ�Ŵ�����ʾ,"E14"
		LED_DISP(led_code, LED_GARBAGE_WARNING, 1);																				//����LED������ʾ,LED_GARBAGE�ڲ�LED��,��ɫ
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag = 0;
	}

	//������״̬���,�������"E15"
	//����δ��Ϊ0,��������Ϊ1,����ʱ�෴
	if(sensor_val[SENS_DUSH_FULL] == 0){
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag = 1;
		MOTOR_SIDE(M_SIDE_Stop);
		MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
		LED_DISP_ERR(led_code, 1,5);																											//������������ʾ,"E15"
		LED_DISP(led_code, LED_GARBAGE_WARNING, 1);																				//����LED������ʾ,LED_GARBAGE�ڲ�LED��,��ɫ
		if(timer5_1s_Dush_Full_flag){
			VOICE_PLAY(VOICE_Play_dong);
			timer5_1s_Dush_Full_flag = 0;
		}else
			;
	}else{
		ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag = 0;
	}
}

//ϵͳ״̬�л�����,������״̬��ʶ����ϵͳ״̬�Զ��л�
void Status_Change_Loop(void)
{
	switch(ROB_Basic_Status_flag){
		case(ROB_Basic_Waiting_flag):																									//����״̬��,���ܽ��е�״̬�л�
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag){
				ROB_Basic_Status_flag = ROB_Basic_Cleaning_flag;															//���ƻ����񵹼�ʱʱ�䵽,�л�����ɨ״̬
				ROB_Sub_Status_Union.ROB_Sub_Status_bit.plan_time_up_flag = 0;
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){
				ROB_Basic_Status_flag = ROB_Basic_Charging_flag;															//����翪ʼ��ʶ��λ,�л������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//����ص�����,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//��˫��̧���ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//������δ�ű�ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//����ص������ڵ���������ֵ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//������¶ȸ����¶ȱ�����ֵ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//����������ʶ��λ,�л�������״̬
			}else
				;
			break;
		case(ROB_Basic_Cleaning_flag):																								//��ɨ״̬��,���ܽ��е�״̬�л�
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Returning_flag;															//����ص�����,�л����Զ�����״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.sys_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//��ϵͳ�쳣��ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//�����ر�ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//��˫��̧���ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_full_flag){
				ROB_Basic_Status_flag = ROB_Basic_Warning_flag;																//����������ʶ��λ,�л�������״̬
			}else
				;
			break;
		case(ROB_Basic_Returning_flag):																								//�Զ�����״̬��,���ܽ��е�״̬�л�
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_very_low_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//����ص�������,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.motor_overload_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//�����ر�ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//��˫��̧���ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag){
				ROB_Basic_Status_flag = ROB_Basic_Charging_flag;															//����翪ʼ��ʶ��λ,�л������״̬
			}else
				;
			break;
		case(ROB_Basic_Charging_flag):																								//���״̬��,���ܽ��е�״̬�л�
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_finished_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//�������ɱ�ʶ��λ,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.power_connected_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//���������е�Դ������,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_start_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//���������е�Դ���عر�,�л�������״̬
			}else
				;
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_temp_high_flag){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//���������е���¶ȹ���,�л�������״̬
			}else
				;
			break;
		case(ROB_Basic_Sleeping_flag):																								//������״̬��,���ܽ��е�״̬�л�

			break;
		case(ROB_Basic_Warning_flag):																									//�ڱ���״̬��,���ܽ��е�״̬�л�
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//��˫��̧���ʶ����,�л�������״̬
			}
			if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.garbage_box_err_flag == 0){
				ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;																//������δ�ű�ʶ����,�л�������״̬
			}
			break;
		default:																																			//Ĭ��ϵͳ���ڴ���״̬
			ROB_Basic_Status_flag = ROB_Basic_Waiting_flag;
			break;
	}
}

//ϵͳ״̬������,�����״̬��ת�ı�ʶ��λ�ڴ˴���
void Status_Loop(void)
{
	switch(ROB_Basic_Status_flag){
		case(ROB_Basic_Waiting_flag):																									//�������״̬
			MOTOR_MOVE_STOP();
			MOTOR_SIDE(M_SIDE_Stop);
			MOTOR_FAN(M_FAN_Stop, M_FAN_speed_stop);
			Navigation_Loop(0,0);
			break;
		case(ROB_Basic_Cleaning_flag):																								//�����ɨ״̬
			if(ROB_Sub_Status_Union.ROB_Sub_Status & 0x00FF){
				Navigation_Loop(0,0);																													//�����쳣,�ر��Զ����ص�������,ֹͣ�������˶�
			}
			else{
				MOTOR_MOVE_RUN(m_move_speed);
			//MOTOR_SIDE(M_SIDE_Run);
			//MOTOR_FAN(M_FAN_Run, m_fan_speed);
				Navigation_Loop(1,0);																													//�����쳣,���ô�ɨ��������
			}
			break;
		case(ROB_Basic_Returning_flag):																								//�����Զ�����״̬
			if(ROB_Sub_Status_Union.ROB_Sub_Status & 0x00FF){
				Navigation_Loop(0,0);																													//�����쳣,�ر��Զ����ص�������,ֹͣ�������˶�
			}else
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.charge_alignment_en_flag)
					Navigation_Loop(0,1);																													//��������Ѷ�׼��ʶ��λ����رձ��Ϲ��ܽ��жԽ�
				else
					Navigation_Loop(1,1);																													//�������δ��׼����򿪱��Ϲ��ܽ��б��Ϸ��ص���
				if(ROB_Sub_Status_Union.ROB_Sub_Status_bit.battery_Low_flag){									//�������ͱ�ʶ��λ������ʾ�����ʹ������
					LED_DISP_ERR(led_code, 1,0);
				}
			break;
		case(ROB_Basic_Charging_flag):																								//������״̬
			Charging_Loop();																															//���ó�紦����
			break;
		case(ROB_Basic_Sleeping_flag):																								//��������״̬

			break;
		case(ROB_Basic_Warning_flag):																									//���뱨��״̬
			Warning_Loop();
			break;
		default:																																			//Ĭ���ڴ���״̬
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
		Sub_Status_Loop();																														//������״̬����������ϵͳ��⣨�ɸ��ݲ�ͬ״̬����Ҫ�ڴ˺�����ʹ��״̬��ʶ���β���Ҫ�ļ�⣩
		Status_Change_Loop();
		Status_Loop();
		Plan_Loop();
		Disp_Loop();
	}
	//return 1;
}

//˫��̧���ⲿ�жϷ������,���ȹرյ��
void EXTI1_IRQHandler(void)
{
	MOTOR_MOVE_STOP();
	MOTOR_SIDE(M_SIDE_Stop);
	MOTOR_FAN(M_FAN_Stop, m_fan_speed);
	ROB_Sub_Status_Union.ROB_Sub_Status_bit.wheel_up_err_flag = 1;
	EXTI->PR &= ~(1<<1);
}

//��ʱ��5�жϷ������,���ڲ�������ʱ���ʶ��Ϊ��ʱ��׼
void TIM5_IRQHandler(void)																										//TIM5�ж�
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  													//���TIM5�����жϷ������
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
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);  																	//���TIM5�����жϱ�־
	}
}
