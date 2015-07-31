#include "led.h"

//u8 CODE_F[]										={0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};							//共阴数码管全显示的代码
//u8 CODE_KEY_SPOT[] 						={0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00};
//u8 CODE_KEY_PLAN[] 						={0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00};
//u8 CODE_KEY_CLEAN[] 					={0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x00};
//u8 CODE_KEY_GARBAGE_NORMAL[] 	={0x00,0x00,0x00,0x00,0x00,0x0E,0x00,0x00};
//u8 CODE_KEY_GARBAGE_WARNING[] ={0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00};
//u8 CODE_KEY_HOME[] 						={0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00};

//通过LED_DIN线向LED控制芯片发送8BIT数据
void LED_Sent_8bits(u8 cmd)
{
	u8 i;
	for(i=0;i<8;i++){
		if(cmd&0x01)
			LED_DIN = 1;
		else
			LED_DIN = 0;
		LED_CLK = 0;
		LED_CLK = 1;
		cmd = cmd>>1;
	}
}

//向LED控制芯片写入命令
void LED_WR(u8 cmd)
{
  LED_STB = 0;
	LED_Sent_8bits(cmd);
	LED_STB = 1;
}

//向LED控制芯片显示地址写入数据
void LED_WR_DATA(u8 *code)
{
	u8 i;
	LED_STB = 0;
	LED_Sent_8bits(0xC0);
	for(i=0;i<8;i++){
		LED_Sent_8bits(code[i]);
		LED_Sent_8bits(0);
  }
  LED_STB = 1;
}

void LED_Init(void)																											//LED控制芯片初始化
{
 u8 CODE_Init[]={0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x00};							//共阴数码管全亮的代码

 GPIO_InitTypeDef  GPIO_InitStructure;

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 								//使能PD端口时钟

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 											//LED_DIN-->PD.8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 									//开漏输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 									//IO口速度为50MHz
 GPIO_Init(GPIOD, &GPIO_InitStructure);					 												//根据设定参数初始化GPIOD.8
 GPIO_ResetBits(GPIOD,GPIO_Pin_8);						 													//PD.8 输出低

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 											//LED_CLK-->PD.9 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 									//开漏输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 									//IO口速度为50MHz
 GPIO_Init(GPIOD, &GPIO_InitStructure);					 												//根据设定参数初始化GPIOD.9
 GPIO_ResetBits(GPIOD,GPIO_Pin_9);						 													//PD.9 输出低

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				 											//LED_STB-->PD.10 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 									//开漏输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 									//IO口速度为50MHz
 GPIO_Init(GPIOD, &GPIO_InitStructure);					 												//根据设定参数初始化GPIOD.10
 GPIO_ResetBits(GPIOD,GPIO_Pin_10);						 													//PD.10 输出低

 LED_WR(0x03);																													//设置显示模式,7位10段模式
 LED_WR(0x40);																													//设置数据命令,采用地址自动加1模式
 LED_WR_DATA(CODE_Init);																								//设置从0x00开始显示的数据

 LED_WR(0x8F);																													//显示控制命令,打开显示并设置为最亮
}

//LED显示函数
void LED_DISP(u8 *code, u8 led_num, u8 disp_num)
{
	u8 i;
	u8 CODE_NUM[]	 ={0xFC,0x60,0xDA,0xF2,0x66,0xB6,0xBE,0xE0,0xFF,0xE6};																			//共阴数码管从"0-9"的代码
	
	if((led_num == 1)||(led_num == 2)||(led_num == 3)||(led_num == 7)){																				//当led_num为LED显示屏上的数字段的时候,在led_num对应位置显示disp_num对应的数字
		if(disp_num == CODE_SPACE)																																								//disp_num等于CODE_SPACE时数码管不显示
			code[led_num] = 0x00;
		else
			code[led_num] = CODE_NUM[disp_num];
		LED_WR_DATA(code);
	}
	else if((led_num == 4)||(led_num == 5)||(led_num == 6)||(led_num == 8)||(led_num == 9)||(led_num == 10)){	//当led_num为LED显示屏上的按键段的时候,在led_num对应位置显示(disp_num不为0)或清除显示(disp_num为0)
		switch(led_num){
			case(LED_SPOT):
				if(disp_num)	code[4] |= CODE_SPOT;
				else					code[4] &= ~CODE_SPOT;
				break;
			case(LED_PLAN):
				if(disp_num)	code[4] |= CODE_PLAN;
				else 					code[4] &= ~CODE_PLAN;
				break;
			case(LED_CLEAN):
				if(disp_num)	code[6] |= CODE_CLEAN;
				else 					code[6] &= ~CODE_CLEAN;
				break;
			case(LED_GARBAGE_NORMAL):
				if(disp_num)	code[5] |= CODE_GARBAGE_NORMAL;
				else					code[5] &= ~CODE_GARBAGE_NORMAL;
				break;
			case(LED_GARBAGE_WARNING):
				if(disp_num)	code[6] |= CODE_GARBAGE_WARNING;
				else					code[6] &= ~CODE_GARBAGE_WARNING;
				break;
			case(LED_HOME):
				if(disp_num)	code[5] |= CODE_HOME;
				else					code[5] &= ~CODE_HOME;
			break;
			default:

			break;
		}
		LED_WR_DATA(code);
	}else if(led_num == 11){																									//当led_num为系统异常状态标识时,显示异常错误代码
		switch(led_num){
			case(LED_ERR_Bat_Low):																								//显示错误代码"Lo"代表电池电量低错误
				if(disp_num){	
					code[2] = CODE_L;  
					code[3] = CODE_o;
				}else{															
					code[2] = 0; 
					code[3] = 0;
				}
				break;
		}
	}else{
		for(i=0;i<8;i++)
			code[i] = 0x00;
	}
}

void LED_DISP_ERR(u8 *code, u8 num2, u8	num1)															//用于LED屏显示错误代码的函数,num2为十位数,num1为个位数,举例：当num2=1,num1=0,显示数据格式为"E10"
{
	if((num2==CODE_SPACE)&&(num1==CODE_SPACE)){
		LED_DISP(code, LED_NUM3, CODE_SPACE);
		LED_DISP(code, LED_NUM2, CODE_SPACE);
		LED_DISP(code, LED_NUM1, CODE_SPACE);
	}else{
//		LED_DISP(code, LED_NUM3, CODE_E);
		code[LED_NUM3] = 0x9E;																									//显示字母"E"
		LED_DISP(code, LED_NUM2, num2);
		LED_DISP(code, LED_NUM1, num1);
	}
}
	
//流水灯显示函数
void LED_Horse_Lamp_Disp(u8 *code, u8 led_i)
{
	switch(led_i){
		case(5):
			LED_DISP(code, LED_NUM1, CODE_SPACE);
			LED_DISP(code, LED_NUM2, CODE_SPACE);
			LED_DISP(code, LED_NUM3, CODE_SPACE);
			LED_DISP(code, LED_NUM4, CODE_SPACE);
			break;
		case(4):
			LED_DISP(code, LED_NUM1, 1);
			LED_DISP(code, LED_NUM2, CODE_SPACE);
			LED_DISP(code, LED_NUM3, CODE_SPACE);
			LED_DISP(code, LED_NUM4, CODE_SPACE);
			break;
		case(3):
			LED_DISP(code, LED_NUM1, 1);
			LED_DISP(code, LED_NUM2, 1);
			LED_DISP(code, LED_NUM3, CODE_SPACE);
			LED_DISP(code, LED_NUM4, CODE_SPACE);
			break;
		case(2):
			LED_DISP(code, LED_NUM1, 1);
			LED_DISP(code, LED_NUM2, 1);
			LED_DISP(code, LED_NUM3, 1);
			LED_DISP(code, LED_NUM4, CODE_SPACE);
			break;
		case(1):
			LED_DISP(code, LED_NUM1, 1);
			LED_DISP(code, LED_NUM2, 1);
			LED_DISP(code, LED_NUM3, 1);
			LED_DISP(code, LED_NUM4, 1);
			break;
		default:
			
			break;
	}
}

//充电电量显示LED流水灯函数
//led_cnt->LED显示方式选择参数,取值1-5
//此函数内部调用了全局变量led_code和g_charge_led_horse_lamp_cnt
void LED_Horse_Lamp(u8 led_cnt)
{
	if(g_charge_led_horse_lamp_cnt){
		g_charge_led_horse_lamp_cnt--;
		LED_Horse_Lamp_Disp(led_code, g_charge_led_horse_lamp_cnt);
	}else{
		g_charge_led_horse_lamp_cnt = led_cnt;
		LED_Horse_Lamp_Disp(led_code, g_charge_led_horse_lamp_cnt);
	}
}

//LED屏显示时间函数,前两位显示小时,后两位显示分钟
void LED_DISP_Time(u8 *code, u8 usr_hour_value, u8 usr_min_value)
{
	u8 temp_plan_hour, temp_hour_l, temp_hour_h;
	u8 temp_plan_min, temp_min_l, temp_min_h;
	
	if(usr_hour_value < 10){
		LED_DISP(code, LED_NUM3, usr_hour_value);
	}else if(usr_hour_value < 24){
		temp_plan_hour = usr_hour_value;
		temp_hour_l = temp_plan_hour % 10;
		temp_plan_hour /= 10;
		temp_hour_h = temp_plan_hour;
		LED_DISP(code, LED_NUM3, temp_hour_l);
		LED_DISP(code ,LED_NUM4, temp_hour_h);
	}else{
		LED_DISP_ERR(code, 4,0);																											//当设置的计划任务小时数大于24小时时报错,错误代码"E40"
	}
	if(usr_hour_value > 24)																													//当设置的计划任务小时数大于24小时时报错,错误代码"E40"
		LED_DISP_ERR(code, 4,0);
	else{																																						//当设置的计划任务小时数小于于24小时时,显示设置分钟数
		if(usr_min_value < 60){
			temp_plan_min = usr_min_value;
			temp_min_l = temp_plan_min % 10;
			temp_plan_min /= 10;
			temp_min_h = temp_plan_min;
			LED_DISP(code, LED_NUM1, temp_min_l);
			LED_DISP(code, LED_NUM2, temp_min_h);
		}else{
			LED_DISP(code, LED_NUM2, 0);
			LED_DISP(code, LED_NUM1, 0);
		}
	}
}
