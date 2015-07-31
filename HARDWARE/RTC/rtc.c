#include "rtc.h"

_CALENDAR_OBJ RTC_Calendar;//ʱ�ӽṹ��

//����RTC�ж�
static void RTC_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;		//RTCȫ���ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	//��ռ���ȼ�0,�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//ʹ�ܸ�ͨ���ж�
	NVIC_Init(&NVIC_InitStructure);		//����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
}

//ʵʱʱ������
//��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������
//BKP->DR1���ڱ����Ƿ��һ�����õ�����
//����0:����
//����:�������
u8 RTC_Init(void)
{
	//����ǲ��ǵ�һ������ʱ��
	u8 temp=0;

//	if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050){																//��ָ���ĺ󱸼Ĵ����ж�������:��������д���ָ�����ݲ����
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);			//ʹ��PWR��BKP����ʱ��
		PWR_BackupAccessCmd(ENABLE);																									//ʹ�ܺ󱸼Ĵ�������
		BKP_DeInit();																																	//��λ��������
//	RCC_LSEConfig(RCC_LSE_ON);																											//�����ⲿ���پ���(LSE),ʹ���ⲿ���پ���
		RCC_HSEConfig(RCC_HSE_ON);																										//�����ⲿ���پ���(LSE),ʹ���ⲿ���پ���
//		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)														//���ָ����RCC��־λ�������,�ȴ����پ������
		while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET){													//���ָ����RCC��־λ�������,�ȴ����پ������
			temp++;
			delay_ms(10);
		}
		if(temp>=250)	return 1;																												//��ʼ��ʱ��ʧ��,����������
//		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);																				//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��
		RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);																//ѡ��HSEʱ��128��Ƶ��ΪRTCʱ��
		RCC_RTCCLKCmd(ENABLE);																												//ʹ��RTCʱ��
		RTC_WaitForLastTask();																												//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		RTC_WaitForSynchro();																													//�ȴ�RTC�Ĵ���ͬ��
		RTC_ITConfig(RTC_IT_SEC | RTC_IT_ALR, ENABLE);																//ʹ��RTC���жϺ������ж�
		RTC_WaitForLastTask();																												//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		RTC_EnterConfigMode();																												//��������
		RTC_SetPrescaler(62500); 																											//����RTCԤ��Ƶ��ֵ,8Mhz/128 = 62500
		RTC_WaitForLastTask();																												//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		RTC_SetAlarm(PLAN_ALARM_CNT_VALUE);																						//����RTC���ӼĴ���ֵ��86400 = 24*3600
		RTC_WaitForLastTask();																												//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		RTC_Set(2015,7,31,16,35,0);  																									//����ʱ��
		RTC_ExitConfigMode(); 																												//�˳�����ģʽ
//		BKP_WriteBackupRegister(BKP_DR1, 0X5050);																			//��ָ���ĺ󱸼Ĵ�����д���û���������
//	}else{																																				//ϵͳ������ʱ
//		RTC_WaitForSynchro();																													//�ȴ����һ�ζ�RTC�Ĵ�����д�������
//		RTC_ITConfig(RTC_IT_SEC, ENABLE);																							//ʹ��RTC���ж�
//		RTC_WaitForLastTask();																												//�ȴ����һ�ζ�RTC�Ĵ�����д�������
//	}
	RTC_NVIC_Config();																														//RCT�жϷ�������
	RTC_Get();																																		//����ʱ��
	return 0; 																																		//ok
}

//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{
	if(year%4==0){ 																																//�����ܱ�4����
		if(year%100==0){
			if(year%400==0)	return 1;																											//�����00��β,��Ҫ�ܱ�400����
			else return 0;
		}else return 1;
	}else return 0;
}

//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�
const u8 table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; 														//���������ݱ�
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};									//ƽ����·����ڱ�

u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	u16 t;
	u32 seccount=0;
	if(syear<1970||syear>2099)return 1;
	for(t=1970;t<syear;t++){																											//��������ݵ��������
		if(Is_Leap_Year(t))seccount+=31622400;																			//�����������
		else seccount+=31536000;			  																						//ƽ���������
	}
	smon-=1;
	for(t=0;t<smon;t++){	   																											//��ǰ���·ݵ����������
		seccount+=(u32)mon_table[t]*86400;																						//�·����������
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;																	//����2�·�����һ���������
	}
	seccount+=(u32)(sday-1)*86400;																								//��ǰ�����ڵ����������
	seccount+=(u32)hour*3600;																											//Сʱ������
    seccount+=(u32)min*60;	 																										//����������
	seccount+=sec;																																//�������Ӽ���ȥ
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);			//ʹ��PWR��BKP����ʱ��
	PWR_BackupAccessCmd(ENABLE);																									//ʹ��RTC�ͺ󱸼Ĵ�������
	RTC_SetCounter(seccount);																											//����RTC��������ֵ

	RTC_WaitForLastTask();																												//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	return 0;
}

//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
u8 RTC_Get(void)
{
	static u16 daycnt=0;
	u32 timecount=0;
	u32 temp_day=0;
	u16 temp_year=0;
	
  timecount=RTC_GetCounter();
 	temp_day=timecount/86400;   																											//�õ�����(��������Ӧ��)
	if(daycnt!=temp_day){																															//����һ����
		daycnt=temp_day;
		temp_year=1970;																																		//��1970�꿪ʼ
		while(temp_day>=365){
			if(Is_Leap_Year(temp_year)){																											//������
				if(temp_day>=366)temp_day-=366;																												//�����������
				else {temp_year++;break;}
			}
			else temp_day-=365;	  																														//ƽ��
			temp_year++;
		}
		RTC_Calendar.w_year=temp_year;																												//�õ����
		temp_year=0;
		while(temp_day>=28)																																//������һ����
		{
			if(Is_Leap_Year(RTC_Calendar.w_year)&&temp_year==1){																	//�����ǲ�������/2�·�
				if(temp_day>=29)temp_day-=29;																													//�����������
				else break;
			}
			else{
				if(temp_day>=mon_table[temp_year])temp_day-=mon_table[temp_year];															//ƽ��
				else break;
			}
			temp_year++;
		}
		RTC_Calendar.w_month=temp_year+1;																											//�õ��·�
		RTC_Calendar.w_date=temp_day+1;  																											//�õ�����
	}
	temp_day=timecount%86400;     																										//�õ�������
	RTC_Calendar.hour=temp_day/3600;     																									//Сʱ
	RTC_Calendar.min=(temp_day%3600)/60; 																									//����
	RTC_Calendar.sec=(temp_day%3600)%60; 																									//����
	RTC_Calendar.week=RTC_Get_Week(RTC_Calendar.w_year,RTC_Calendar.w_month,RTC_Calendar.w_date);	//��ȡ����
	return 0;
}

//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//�������:����������
//����ֵ:���ں�
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{
	u16 temp2;
	u8 yearH,yearL;

	yearH=year/100;	yearL=year%100;
// ���Ϊ21����,�������100																																								
	if (yearH>19)yearL+=100;
// ����������ֻ��1900��֮���																																								
	temp2=yearL+yearL/4;
	temp2=temp2%7;
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}

