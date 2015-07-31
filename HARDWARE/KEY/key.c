#include "key.h"

////������ʼ������
//void KEY_Init(void) //IO��ʼ��SPOT-->GPIOB.5,CLEAN-->GPIOB.6,PLAN-->GPIOB.7,HOME-->GPIOB.8 ��������
//{
// 	GPIO_InitTypeDef GPIO_InitStructure;

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTBʱ��

//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;//PB5~8
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB5,6,7,8
//}

void KEY_Init(void)	//IO��ʼ��SPOT-->GPIOB.5,CLEAN-->GPIOB.6,PLAN-->GPIOB.7,HOME-->GPIOB.8 ��������,�жϷ�ʽ
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);						//ʹ�ܸ��ù���ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);					//ʹ��GPIOBʱ��
	
	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

//GPIOB �ж����Լ��жϳ�ʼ������   �½��ش���
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource5);

  EXTI_InitStructure.EXTI_Line 		= EXTI_Line5;																											//GPIOB.5
  EXTI_InitStructure.EXTI_Mode 		= EXTI_Mode_Interrupt;																						//����Ϊ�ж�ģʽ
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;																						//�ж��½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 																																	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource6);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line6;	
	EXTI_Init(&EXTI_InitStructure);	 
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource7);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line7;	
	EXTI_Init(&EXTI_InitStructure);	 
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);
	EXTI_InitStructure.EXTI_Line 		= EXTI_Line8;	
	EXTI_Init(&EXTI_InitStructure);	 

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;																								//ʹ��GPIOB.5|GPIOB.6|GPIOB.7|GPIOB.8���ڵ��ⲿ�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;																			//��ռ���ȼ�2,
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;																							//�����ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;																										//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);
}

////����������(�жϷ�ʽ)
//u8 KEY_Scan(void)
//{
//	if(EXTI_Key_flag)
//	{
//		EXTI_Key_flag = 0;
//		return KeySta;
//	}
// 	return 0;// �ް�������
//}

////����������(��ѯ��ʽ)
////���ذ���ֵ
////mode:0,��֧��������;1,֧��������;
////0,û���κΰ�������
////1,KEY_SPOT����
////2,KEY_CLEAN����
////3,KEY_PLAN����
////4,KEY_HOME����
////ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>KEY3!!
//u8 KEY_Scan(u8 mode)
//{
//	static u8 key_up=1;																	//�������ɿ���־
//	if(mode)	key_up=1;  																//֧������
//	
//	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==0))
//	{
//		key_up=0;
//		if(KEY0==0)return KEY_SPOT;
//		else if(KEY1==0)return KEY_CLEAN;
//		else if(KEY2==0)return KEY_PLAN;
//		else if(KEY3==0)return KEY_HOME;
//	}else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1)key_up=1;
// 	return 0;// �ް�������
//}
