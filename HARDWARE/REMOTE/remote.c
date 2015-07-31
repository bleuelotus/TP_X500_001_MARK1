#include "remote.h"

//ң��������״̬
//[7]:�յ����������־
//[6]:�õ���һ��������������Ϣ
//[5]:����
//[4]:����������Ƿ��Ѿ�������
//[3:0]:�����ʱ��
u8 	RmtSta=0;
u16 Dval;				//�½���ʱ��������ֵ
u32 RmtRec=0;		//������յ�������
u8  RmtCnt=0;		//�������µĴ���
u8	RmtRec_Cnt = 0;	//���ڼ�¼���յ������ݵ�λ���Ա��ж��Ƿ�������
u8 	EXTI_Rmt_flag;

//�Զ����س�����������״̬
//[7]:�յ����������־
//[6]:�õ���һ��������������Ϣ
//[5]:����
//[4]:����������Ƿ��Ѿ�������
//[3:0]:�����ʱ��
u8 	Rtrn_RmtSta=0;
u16 Rtrn_Dval;						//�½���ʱ��������ֵ
u32 Rtrn_RmtRec=0;				//������յ�������
u8  Rtrn_RmtCnt=0;				//�������µĴ���
u8	Rtrn_RmtRec_Cnt = 0;	//���ڼ�¼���յ������ݵ�λ���Ա��ж��Ƿ�������
u8 	Rtrn_EXTI_Rmt_flag;		


//ͨ�ö�ʱ��2�жϳ�ʼ��,����ͳ�ƺ�������жϴ������ʱ��
//����ʱ��ѡ��ΪAPB1��2��,��APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
void TIM2_Int_Init(u16 arr,u16 psc)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��

	//��ʱ��TIM5��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIM2
}

//������ն˿ڳ�ʼ��
void REMOTE_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM2_Int_Init(65535, 71);																			//��ʼ����ʱ��2��Ϊ������մ�����ʱ��,ÿ1usTIM2��������1

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);						//ʹ�ܸ��ù���ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);					//ʹ��GPIODʱ��

//GPIOD.11 �ж����Լ��жϳ�ʼ������   �����ش���
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource11);

	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;
	GPIO_Init(GPIOD,&GPIO_InitStructure);

  EXTI_InitStructure.EXTI_Line = EXTI_Line11;										//GPIOD.11
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;						//����Ϊ�ж�ģʽ
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;				//�ж������ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 															//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;					//ʹ��GPIOD.11���ڵ��ⲿ�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2,
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);
}

//�Զ����غ�����ն˿ڳ�ʼ��
void RETURN_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);						//ʹ�ܸ��ù���ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);					//ʹ��GPIODʱ��

	GPIO_InitStructure.GPIO_Pin 		= GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode 		= GPIO_Mode_IPU;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	
	//GPIOD.12�ж����Լ��жϳ�ʼ������   �����ش���
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource12);

  EXTI_InitStructure.EXTI_Line = EXTI_Line12;										//GPIOD.12
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;						//����Ϊ�ж�ģʽ
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;				//�ж������ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 															//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
	
	//GPIOD.13�ж����Լ��жϳ�ʼ������   �����ش���
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource13);
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;										//GPIOD.13
	EXTI_Init(&EXTI_InitStructure);

	//GPIOD.14�ж����Լ��жϳ�ʼ������   �����ش���
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource14);
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;										//GPIOD.14
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOD.15�ж����Լ��жϳ�ʼ������   �����ش���
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource15);
	EXTI_InitStructure.EXTI_Line = EXTI_Line15;										//GPIOD.15
	EXTI_Init(&EXTI_InitStructure);
}

//��������жϴ���
void EXTI15_10_IRQHandler(void)
{
//	EXTI->IMR &= ~(1<<11); 																		//��������LINE11(GPIOD.11)�ϵ��ж�����

 if(EXTI_GetFlagStatus(EXTI_Line11)!=RESET){
		if(RDATA){																							//�����ز���
	    TIM_SetCounter(TIM2,0);	   															//��ն�ʱ������ֵ
			TIM_Cmd(TIM2, ENABLE);																	//ʹ�ܶ�ʱ��2��ʼ��ʱ
			EXTI->RTSR &= ~(1<<11);																	//��ֹ��������ж������ش���ģʽ
			EXTI->FTSR |= (1<<11);																	//���ú�������ж�Ϊ�½��ش���ģʽ
			RmtSta|=0x10;																						//����������Ѿ�������
		}else{ 																									//�½��ز���
			Dval = TIM2->CNT;																				//��ȡTIM2������ֵ
			TIM_Cmd(TIM2, DISABLE);																	//ֹͣ��ʱ��2��ʱ
			EXTI->FTSR &= ~(1<<11);																	//��ֹ��������ж��½��ش���ģʽ
			EXTI->RTSR |= (1<<11);																	//���ú�������ж�Ϊ�����ش���ģʽ

			if(RmtSta&0x10){																				//���һ�θߵ�ƽ����
 				if(RmtSta&0x80){																			//���յ���������
					if(Dval>300&&Dval<800){																//560Ϊ��׼ֵ,560us
						RmtRec<<=1;																						//����һλ.
						RmtRec|=0;																							//���յ�0
						RmtRec_Cnt++;																						//RmtRec���յ�������λ������һλ
					}else if(Dval>1400&&Dval<1800){												//1680Ϊ��׼ֵ,1680us
						RmtRec<<=1;																							//����һλ.
						RmtRec|=1;																							//���յ�1
						RmtRec_Cnt++;																						//RmtRec���յ�������λ������һλ
					}else if(Dval>2200&&Dval<2600){												//�õ�������ֵ���ӵ���Ϣ 2500Ϊ��׼ֵ2.5ms
						RmtCnt++; 																							//������������1��
						RmtSta&=0xF0;																						//��ռ�ʱ��
					}
 				}else if(Dval>4200&&Dval<4700){													//4500Ϊ��׼ֵ4.5ms
					RmtSta|=1<<7;																						//��ǳɹ����յ���������
					RmtCnt=0;																								//�����������������
				}
			}
			RmtSta&=~(1<<4);
		}
	}
	if(RmtRec_Cnt == 32){
		if(EXTI_GetFlagStatus(EXTI_Line11)!=RESET){
			if(RmtSta&0x80){															//�ϴ������ݱ����յ���
				RmtSta&=~0x10;															//ȡ���������Ѿ���������
				if((RmtSta&0x0F)==0x00)
					RmtSta|=1<<6;															//����Ѿ����һ�ΰ����ļ�ֵ��Ϣ�ɼ�
				if((RmtSta&0x0F)<14)
					RmtSta++;																	//�����ʱ��++
				else{
					RmtSta&=~(1<<7);													//���������ʶ
					RmtSta&=0xF0;															//��ռ�����
				}
			}
		}
		RmtRec_Cnt = 0;
		EXTI_Rmt_flag = 1;
	}
//	EXTI->IMR |= (1<<11);																				//ʹ��GPIOD.11�ж���
	EXTI->PR  |= (1<<11);																				//���LINE11�ϵ��жϱ�ʶ
}

//����������
//����ֵ:
//	 0,û���κΰ�������
//����,���µİ�����ֵ.
u8 REMOTE_Scan(void)
{
	u8 sta=0;
  u8 t1,t2;
	if(EXTI_Rmt_flag){
		if(RmtSta&(1<<6)){//�õ�һ��������������Ϣ��
			t1=RmtRec>>24;			//�õ���ַ��
			t2=(RmtRec>>16)&0xff;	//�õ���ַ����
			if((t1==(u8)~t2)&&t1==REMOTE_ID){//����ң��ʶ����(ID)����ַ
				t1=RmtRec>>8;
				t2=RmtRec;
				if(t1==(u8)~t2)sta=t1;//��ֵ��ȷ
			}
			if((sta==0)||((RmtSta&0x80)==0)){//�������ݴ���/ң���Ѿ�û�а�����
				RmtSta&=~(1<<6);//������յ���Ч������ʶ
				RmtCnt=0;		//�����������������
			}
		}
		EXTI_Rmt_flag = 0;
	}else
//		if(RmtCnt);								//���RmtCnt����0,��˵�������ظ�����,sta�����ϴζ�ȡֵ
//		else{
			sta = 0;
//		}
//	if(EXTI_Rmt_flag){
//		printf("RmtRec_Cnt is:%d\r\n",RmtRec_Cnt);
//		printf("sta is:%d\r\n",sta);
//		exti_flag = 0;
//	}
  return sta;
}

//�Զ����س���������źŴ���
//����ֵ:
//	 0,û���κΰ�������
//����,���µİ�����ֵ.
u8 RETURN_Scan(void)
{
	u8 sta=0;
  u8 t1,t2;
	if(Rtrn_EXTI_Rmt_flag){
		if(Rtrn_RmtSta&(1<<6)){//�õ�һ��������������Ϣ��
			t1=Rtrn_RmtRec>>24;			//�õ���ַ��
			t2=(Rtrn_RmtRec>>16)&0xff;	//�õ���ַ����
			if((t1==(u8)~t2)&&t1==REMOTE_ID){//����ң��ʶ����(ID)����ַ
				t1=Rtrn_RmtRec>>8;
				t2=Rtrn_RmtRec;
				if(t1==(u8)~t2)sta=t1;//��ֵ��ȷ
			}
			if((sta==0)||((Rtrn_RmtSta&0x80)==0)){//�������ݴ���/ң���Ѿ�û�а�����
				Rtrn_RmtSta&=~(1<<6);//������յ���Ч������ʶ
				RmtCnt=0;		//�����������������
			}
		}
		Rtrn_EXTI_Rmt_flag = 0;
	}else
//		if(RmtCnt);								//���RmtCnt����0,��˵�������ظ�����,sta�����ϴζ�ȡֵ
//		else{
			sta = 0;
//		}
//	if(EXTI_Rmt_flag){
//		printf("RmtRec_Cnt is:%d\r\n",RmtRec_Cnt);
//		printf("sta is:%d\r\n",sta);
//		exti_flag = 0;
//	}
	
  return sta;
}
