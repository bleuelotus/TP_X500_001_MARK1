#ifndef __LED_H
#define __LED_H	 

#include "usr_includes.h"

//LED��ʾ����
/******************************************************************************************************************************
							Byte0, Byte1, Byte2, Byte3, Byte4, Byte5, Byte6, Byte7
CODE_LED[] = {0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00};

LED->SPOT		=>Byte4->bit.6(D)		,bit.7(UR)		,bit.8(UL)                 	��λ�ù�������LED,�ֲ�Ϊ��,����,����
LED->PLAN		=>Byte4->bit.2(D)		,bit.3(UR)		,bit.4(UL)									��λ�ù�������LED,�ֲ�Ϊ��,����,����
LED->CLEAN	=>Byte6->bit.2(DR)	,bit.3(DL)		,bit.4(UR)		,bit.5(UL)		��λ�ù����ĸ�LED,�ֲ�Ϊ����,����,����,����
LED->GARBAGE=>Byte5->bit.2(O_D)	,bit.3(O_UR)	,bit.4(O_UL)								��λ�ù������������LED,	�ֲ���Ϊ��,����,����,���Ϊ��ɫ,
							Byte6->bit.6(I_D)	,bit.7(I_UR)	,bit.8(I_UL)																					�ֲ���Ϊ��,����,����,�ڲ�Ϊ��ɫ
LED->HOME		=>Byte5->bit.5(DR)	,bit.6(DL)		,bit.7(UR)		,bit.8(UL)		��λ�ù����ĸ�LED,�ֲ�Ϊ����,����,����,����

			Byte7   Byte1				Byte2		Byte3
			* * *		* * *				*	*	*		* * *			* 7 *
			*		*	  *		*		*		*		*		*		*			2		6
			* * *		* * *		*		*	*	*		* * *	<=	*	1	*  0
		  *		*   *		*		*		*		*   * 	*			3		5
			* * *		* * *				*	* *		* * *			*	4 *
				
LED->NUM1		=>Byte3
LED->NUM2		=>Byte2
LED->NUM3		=>Byte1
LED->NUM4		=>Byte7
*******************************************************************************************************************************/

void LED_Init(void);																													//��ʼ��LED��ʾ��
void LED_WR(u8 cmd);																													//LED����оƬ����д��
void LED_WR_DATA(u8 *code); 																									//��LED����оƬ��ʾ��ַд������
void LED_DISP(u8 *code, u8 led_num, u8 disp_num); 														//LED��ʾ����ʾ����
void LED_DISP_ERR(u8 *code, u8 num2, u8	num1);																//����LED����ʾ�������ĺ���,num2Ϊʮλ��,num1Ϊ��λ��,��������num2=1,num1=0,��ʾ���ݸ�ʽΪ"E10"
void LED_Horse_Lamp_Disp(u8 *code, u8 led_i);																	//��ˮ����ʾ����
void LED_Horse_Lamp(u8 led_cnt);																							//��������ʾLED��ˮ�ƺ���
void LED_DISP_Time(u8 *code, u8 usr_hour_value, u8 usr_min_value);						//LED����ʾʱ�亯��,ǰ��λ��ʾСʱ,����λ��ʾ����
		 				    
#endif
