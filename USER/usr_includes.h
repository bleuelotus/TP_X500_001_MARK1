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

//�����˻���״̬����
#define ROB_Basic_Waiting_flag  					1																		//����״̬��ʶ
#define ROB_Basic_Cleaning_flag 					2																		//��ɨ״̬��ʶ
#define ROB_Basic_Returning_flag 					3																		//�Զ����س����״̬��ʶ
#define ROB_Basic_Charging_flag						4																		//���״̬��ʶ
#define ROB_Basic_Sleeping_flag 					5																		//����״̬��ʶ
#define ROB_Basic_Warning_flag						6																		//����״̬��ʶ

//PWM��س���
#define TIM8_PWM_ARR_Value								99																	//99->{(99+1)*(71+1)/72000000 = 0.0001s(10Khz)},999->{(999+1)*(71+1)/72000000 = 0.001s(1Khz)}
#define TIM8_PWM_PSC_Value								71

//���Ϸ���LEDǿ��ֵ
#define	SENS_LED_Value_const							42																	//���Ϸ���LEDǿ��ֵ,����Ϊ42����ʱ�������
#define	SENS_LED_Value_max 								90																	//���Ϸ���LEDǿ�����ֵ

//���ߵ���ж�������
#define M_MOVE_Forward										0
#define M_MOVE_Backward										1
#define M_MOVE_ClkWiz											2
#define	M_MOVE_AntiClkWiz									3
#define M_MOVE_TurnLeft										4
#define	M_MOVE_TurnRight									5

//���ߵ����ͣ����
#define M_MOVE_Run												0
#define	M_MOVE_Stop												1

//���ߵ���ٶȳ���
#define M_MOVE_speed_h										70
#define M_MOVE_speed_m										40
#define M_MOVE_speed_l										10

//���ߵ�����ƶ˿ں�_���
#define M_MOVE_DIREC_L 										PEout(1)														//PE1	->��������ת����,	0��ת,1��ת
#define M_MOVE_STOP_L 										PEout(2)														//PE2	->������ͣ����,		0����,1ֹͣ
#define M_MOVE_DIREC_R 										PEout(9)														//PE9	->�ҵ������ת����,	0��ת,1��ת
#define M_MOVE_STOP_R 										PEout(10)														//PE10->�ҵ����ͣ����,		0����,1ֹͣ

//��ˢ�����ͣ����
#define M_SIDE_Run												1
#define M_SIDE_Stop												0

//��ˢ������ƶ˿ں�_���
#define M_SIDE_L 													PEout(5)														//PE5
#define M_SIDE_R 													PEout(11)														//PE11

//�����ͣ����
#define M_FAN_Run													1
#define M_FAN_Stop												0

//����ٶȳ���
#define M_FAN_speed_h											99
#define M_FAN_speed_m											60
#define M_FAN_speed_l											20
#define M_FAN_speed_stop									0

//LED��ʾ�����ƶ˿ں�_���
#define	LED_DIN 													PDout(8)														//PD8
#define	LED_CLK 													PDout(9)														//PD9
#define	LED_STB 													PDout(10)														//PD10

//LED��ʾ��Ӧ��ʾ�����е�λ��,NUM1-NUM4Ϊ���ҵ���������
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
#define LED_ERR_Bat_Low										11																	//��ʾ��ص����ʹ������

//LED_���ݰ����µ�LED����ʾ����
#define CODE_SPACE												11																	//LED����
#define CODE_SPOT  												0xE0																//���ݰ���SPOT�µ�LED����
#define CODE_PLAN 												0x0E																//���ݰ���PLAN�µ�LED����
#define CODE_CLEAN  											0x1E																//���ݰ���CLEAN�µ�LED����
#define CODE_GARBAGE_NORMAL	 							0x0E																//���ݰ�������Ͱ�µ�LED����_��Ȧ_��ɫ
#define CODE_GARBAGE_WARNING  						0xE0																//���ݰ�������Ͱ�µ�LED����_��Ȧ_��ɫ
#define CODE_HOME  												0xF0																//���ݰ���HOME�µ�LED����
#define CODE_E 														10																	//��ĸ"E",����LED_DISP�����������е���code[10]=0x9E
#define CODE_L														0x1C																//��ĸ"L"
#define CODE_o														0x3A																//��ĸ"o"

//����оƬ���ƶ˿ں�_���
#define	VOICE_RST PEout(3)																										//VOICE_RST->PE.3
#define VOICE_DIN	PEout(4)																										//VOICE_DIN->PE.4

//������Ӧ��ַ��
#define VOICE_Play_charging_up_en					0x00																//Ӣ��->"Charing up"
#define VOICE_Play_charging_start_en			0x01																//Ӣ��->"Charing start"
#define VOICE_Play_set_sure								0x02																//Ӣ��->"Set sure"????�˴���ȷ��,������
#define VOICE_Play_ding										0x03																//��Ч->"��"
#define VOICE_Play_dong										0x04																//��Ч->"��"
#define VOICE_Play_charging_up_ch					0x05																//����->"�����ѳ���"
#define VOICE_Play_remind_long						0x06																//��Ч->����ʾ��
#define VOICE_Play_charging_start_ch			0x07																//����->"��ʼ���"
#define VOICE_Play_remind_short						0x08																//��Ч->����ʾ��
#define VOICE_Play_set_success						0x0A																//����->"���óɹ�"

//���ݰ������ƶ˿ں�_����
#define KEY0 															PBin(5)  														//��ȡ����0_SPOT
#define KEY1 															PBin(6)															//��ȡ����1_CLEAN
#define KEY2 															PBin(7)															//��ȡ����2_PLAN
#define KEY3 															PBin(8)															//��ȡ����3_HOME

//���ݰ�����ز���
#define KEY_SPOT													1
#define KEY_CLEAN													2
#define KEY_PLAN													3
#define KEY_HOME													4

//�ƻ�������ز���
#define PLAN_ALARM_CNT_VALUE							86400

//���״̬��س���
#define SYS_Battery_Temp_High							2000																//����¶ȸ߱�����ֵ
#define SYS_Battery_Low										1750																//��ص����ͱ�����ֵ,���ڴ�ֵ��������л����Զ����״̬
#define SYS_Battery_Very_Low							1650																//��ص������ͱ�����ֵ,���ڴ�ֵ��Ϊ�������ֹͣ�Զ����ع���,��������ʾ�ֶ����
#define SYS_Power24V_Connected_Value			2000																//24V��Դ�����ж���ֵ
#define SYS_Power24V_Switch_err_Value			1500																//��Դ����δ���ж���ֵ,��24V��Դ����ʱ,���˿���δ��,PC5�϶�ȡ�ĵ�ص���ADCֵ�ӽ�0
#define SYS_Power24V_Switch_off_err_Value	2600																//�������е�Դ���عر��ж���ֵ,����ѹ���ڴ�ֵ��,֤����Դ���عر�
#define SYS_Battery_Full									2150																//��س�������ֵ��						 14.4V*1.15 	= 16.56V  Value_adc = (16.56*56/(56+470))*4096/3.3 = 2188.3
#define SYS_Battery_1d4										1850																//��ص����ﵽ1/4�����ж���ֵ, 14.4V*0.85 	= 12.24V  Value_adc = (12.24*56/(56+470))*4096/3.3 = 12.24*132.14425 = 1617
#define SYS_Battery_2d4										1950																//��ص����ﵽ2/4�����ж���ֵ, 14.4V*0.925 	= 13.32V  Value_adc = (13.32*56/(56+470))*4096/3.3 = 13.32*132.14425 = 1760
#define SYS_Battery_3d4										2050																//��ص����ﵽ3/4�����ж���ֵ, 14.4V								  Value_adc = 14.4*132.14425 = 1903

//������س���
#define SENS_F_L_EN_Value									2500																//ǰ�����󴫸���ʹ����ֵ
#define SENS_F_LM_EN_Value								3200																//ǰ�������д�����ʹ����ֵ
#define SENS_F_M_EN_Value									3400																//ǰ�����д�����ʹ����ֵ
#define SENS_F_RM_EN_Value								3300																//ǰ�������д�����ʹ����ֵ
#define SENS_F_R_EN_Value									3700																//ǰ�����Ҵ�����ʹ����ֵ

#define SENS_F_L_nEN											(sensor_val[0]>SENS_F_L_EN_Value)
#define SENS_F_LM_nEN											(sensor_val[1]>SENS_F_LM_EN_Value)
#define SENS_F_M_nEN											(sensor_val[2]>SENS_F_M_EN_Value)
#define SENS_F_RM_nEN											(sensor_val[3]>SENS_F_RM_EN_Value)
#define SENS_F_R_nEN											(sensor_val[4]>SENS_F_R_EN_Value)

//��ײ��س���
#define SENS_CRUSH_EN											0

#define SENS_CRUSH_L_nEN									(sensor_val[5] 	!= SENS_CRUSH_EN)
#define SENS_CRUSH_R_nEN									(sensor_val[6] 	!= SENS_CRUSH_EN)

//̽����س���,����ʱSENS_B_ENΪ0��ʵ��ʹ����Ϊ1
#define SENS_B_EN													1

#define SENS_B_L_nEN											(sensor_val[7] 	!= SENS_B_EN)
#define SENS_B_LM_nEN											(sensor_val[8] 	!= SENS_B_EN)
#define SENS_B_RM_nEN											(sensor_val[9] 	!= SENS_B_EN)
#define SENS_B_R_nEN											(sensor_val[10] != SENS_B_EN)

//��������س���
#define SENS_F_L													0
#define SENS_F_L_ADC_CH										6
#define SENS_F_L_Value 										PAin(6)  														//PA6		1		ǰ������

#define SENS_F_LM													1
#define SENS_F_LM_ADC_CH									3
#define SENS_F_LM_Value 									PAin(3)  														//PA3		2		ǰ��������

#define SENS_F_M													2
#define SENS_F_M_ADC_CH										4
#define SENS_F_M_Value 										PAin(4)  														//PA4		3		ǰ������

#define SENS_F_RM													3
#define SENS_F_RM_ADC_CH									7
#define SENS_F_RM_Value 									PAin(7)  														//PA7		4		ǰ��������

#define SENS_F_R													4
#define SENS_F_R_ADC_CH										5
#define SENS_F_R_Value 										PAin(5)  														//PA5		5		ǰ������

#define SENS_FI_L													5
#define SENS_FI_L_Value 									PBin(14)  													//PB14	6		ǰ������ײ��
#define SENS_FI_R													6
#define SENS_FI_R_Value 									PBin(15)  													//PB15	7		ǰ������ײ��

#define SENS_B_L													7
#define SENS_B_L_Value  									PEin(12)  													//PE12	1		̽�׼����
#define SENS_B_LM													8
#define SENS_B_LM_Value 									PEin(13)  													//PE13	2		̽�׼������
#define SENS_B_RM													9
#define SENS_B_RM_Value 									PEin(15)														//PE15	3		̽�׼������
#define SENS_B_R													10
#define SENS_B_R_Value  									PEin(14)														//PE14	4		̽�׼����

#define SENS_WHEEL_UP											11
#define SENS_WHEEL_UP_Value 							PAin(1)															//PA1		1		˫��̧��
#define SENS_NO_GARBAGE_BOX								12
#define SENS_NO_GARBAGE_BOX_Value					PCin(12)														//PC12	2		������δ�ż��
#define SENS_DUSH_FULL										13
#define SENS_DUSH_FULL_Value							PCin(10)														//PC10	3		�ҳ�������

#define SENS_CHARGE_BOX_CON								14
#define SENS_CHARGE_BOX_CON_Value 				PAin(2)															//PA2		1		��������Ӽ��
#define SENS_CB_LOC_L											15
#define SENS_CB_LOC_L_Value 							PDin(15)														//PD15	2		�������λ������
#define SENS_CB_LOC_LM										16
#define SENS_CB_LOC_LM_Value 							PDin(13)														//PD13	3		�������λ����
#define SENS_CB_LOC_RM										17
#define SENS_CB_LOC_RM_Value 							PDin(14)														//PD14	4		�������λ����
#define SENS_CB_LOC_R											18
#define SENS_CB_LOC_R_Value								PDin(12)														//PD12	5		�������λ������

//����Ϊ0,����Ϊ1
#define SENS_FAN_OVLOAD										19
#define SENS_FAN_OVLOAD_Value							PDin(3)															//PD3		1		����������ؼ��
#define SENS_M_SIDE_L_OVLOAD							20
#define SENS_M_SIDE_L_OVLOAD_Value				PDin(4)															//PD4		2		���ˢ������ؼ��
#define SENS_M_SIDE_R_OVLOAD							21
#define SENS_M_SIDE_R_OVLOAD_Value				PDin(5)															//PD5		3		�ұ�ˢ������ؼ��
#define SENS_M_MOVE_L_OVLOAD							22
#define SENS_M_MOVE_L_OVLOAD_Value				PDin(6)															//PD6		4		�����ߵ�����ؼ��
#define SENS_M_MOVE_R_OVLOAD							23
#define SENS_M_MOVE_R_OVLOAD_Value				PDin(7)															//PD7		5		�����ߵ�����ؼ��

#define SENS_Battery											24
#define SENS_Battery_ADC_CH								15
#define SENS_Battery_Value								PCin(5)															//PC5		1		��ص������

#define SENS_24V													25
#define SENS_Charging_Battery_ADC_CH			14
#define SENS_Charging_Battery_Value				PCin(4)															//PC4		2		���ʱ��Դ�������

#define SENS_Battery_temp									26
#define SENS_Battery_temp_ADC_CH					10
#define SENS_Battery_temp_Value						PCin(0)															//PC0		3		����¶ȼ��

//ȫ�ֱ�������
extern u8 g_charge_led_horse_lamp_cnt;
extern u8 g_charge_start_flag;
extern const u16 m_move_length_r;
extern const u16 m_move_length_l;
extern const u16 m_move_encoder_num;
extern u8 led_code[];
extern u8	g_RTC_1s_charging_led_flag;
