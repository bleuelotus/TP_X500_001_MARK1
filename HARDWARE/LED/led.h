#ifndef __LED_H
#define __LED_H	 

#include "usr_includes.h"

//LED显示控制
/******************************************************************************************************************************
							Byte0, Byte1, Byte2, Byte3, Byte4, Byte5, Byte6, Byte7
CODE_LED[] = {0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00};

LED->SPOT		=>Byte4->bit.6(D)		,bit.7(UR)		,bit.8(UL)                 	此位置共有三个LED,分布为下,右上,左上
LED->PLAN		=>Byte4->bit.2(D)		,bit.3(UR)		,bit.4(UL)									此位置共有三个LED,分布为下,右上,左上
LED->CLEAN	=>Byte6->bit.2(DR)	,bit.3(DL)		,bit.4(UR)		,bit.5(UL)		此位置共有四个LED,分布为右下,左下,右上,左上
LED->GARBAGE=>Byte5->bit.2(O_D)	,bit.3(O_UR)	,bit.4(O_UL)								此位置共有里外各三个LED,	分布都为下,右上,左上,外层为紫色,
							Byte6->bit.6(I_D)	,bit.7(I_UR)	,bit.8(I_UL)																					分布都为下,右上,左上,内层为红色
LED->HOME		=>Byte5->bit.5(DR)	,bit.6(DL)		,bit.7(UR)		,bit.8(UL)		此位置共有四个LED,分布为右下,左下,右上,左上

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

void LED_Init(void);																													//初始化LED显示屏
void LED_WR(u8 cmd);																													//LED控制芯片命令写入
void LED_WR_DATA(u8 *code); 																									//向LED控制芯片显示地址写入数据
void LED_DISP(u8 *code, u8 led_num, u8 disp_num); 														//LED显示屏显示数字
void LED_DISP_ERR(u8 *code, u8 num2, u8	num1);																//用于LED屏显示错误代码的函数,num2为十位数,num1为个位数,举例：当num2=1,num1=0,显示数据格式为"E10"
void LED_Horse_Lamp_Disp(u8 *code, u8 led_i);																	//流水灯显示函数
void LED_Horse_Lamp(u8 led_cnt);																							//充电电量显示LED流水灯函数
void LED_DISP_Time(u8 *code, u8 usr_hour_value, u8 usr_min_value);						//LED屏显示时间函数,前两位显示小时,后两位显示分钟
		 				    
#endif
