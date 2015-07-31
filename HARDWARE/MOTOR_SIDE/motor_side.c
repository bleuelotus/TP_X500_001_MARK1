#include "motor_side.h"

//边刷电机控制端口初始化函数
void MOTOR_SIDE_Init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 	//使能PE端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 				//左边刷电机控制端口-->PE.5 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 					//根据设定参数初始化GPIOE.5
 GPIO_ResetBits(GPIOE,GPIO_Pin_5);						 						//PE.5 输出低
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 				//右边刷电机控制端口-->PE.11 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 		//推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 		//IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 					//根据设定参数初始化GPIOE.11
 GPIO_ResetBits(GPIOE,GPIO_Pin_11);						 						//PE.11 输出低
}

void MOTOR_SIDE(u8 m_side_run)														//边刷电机启停控制,m_side_run->1运行,0停止
{
	if(m_side_run){
		M_SIDE_L = M_SIDE_Run;
		M_SIDE_R = M_SIDE_Run;
	}else{
		M_SIDE_L = M_SIDE_Stop;
		M_SIDE_R = M_SIDE_Stop;
	}
}
 
