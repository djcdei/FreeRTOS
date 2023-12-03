#include "includes.h"

void rtc_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	RTC_InitTypeDef RTC_InitStructure;
	// ʹ��rtc��Ӳ��ʱ��

	/* Enable the PWR clock��ʹ�ܵ�Դ����ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC���������RTC */
	PWR_BackupAccessCmd(ENABLE);

#if 0
	//��LSE��ʱ��
	RCC_LSEConfig(RCC_LSE_ON);

	//���LSE��ʱ���Ƿ����
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	
	//ΪRTCѡ��LSE��Ϊʱ��Դ
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#else
	/*ʹ��LSIʱ�ӣ�32KHz��*/
	RCC_LSICmd(ENABLE);
	/*�ȴ�LSTʱ��ʹ�ܳɹ� Wait till LSI is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		;
	/*ѡ��ʱ��ԴLSI*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

#endif

	/*����RTC���Ƶ��1Hz-->����Ԥ��Ƶֵ*/
	/* RTCʱ�Ӽ��㷽ʽck_spre(1Hz) = RTCCLK(LSE 32Khz) /(uwAsynchPrediv + 1)/(uwSynchPrediv + 1)*/
	/* Enable the RTC Clock ��ʹ��RTCʱ��*/
	RCC_RTCCLKCmd(ENABLE);
	/* Wait for RTC APB registers synchronisation ���ȴ�RTC��ؼĴ�������*/
	RTC_WaitForSynchro();

	/* Configure the RTC data register and RTC prescaler������RTC���ݼĴ�����RTC�ķ�Ƶֵ */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;			  // �첽��Ƶϵ�� 128-1
	RTC_InitStructure.RTC_SynchPrediv = 0xF9;			  // ͬ����Ƶϵ�� 250-1
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; // 24Сʱ��ʽ
	RTC_Init(&RTC_InitStructure);

	// ��������
	RTC_DateStructure.RTC_Year = 0x23;					   // 2023��
	RTC_DateStructure.RTC_Month = RTC_Month_November;	   // 11�·�
	RTC_DateStructure.RTC_Date = 0x28;					   // ��28��/28��/28��
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Tuesday; // ����2
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

	// ����ʱ�� 17:14:50
	RTC_TimeStructure.RTC_H12 = RTC_H12_PM;
	RTC_TimeStructure.RTC_Hours = 0x11;
	RTC_TimeStructure.RTC_Minutes = 0x10;
	RTC_TimeStructure.RTC_Seconds = 0x50;
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

	// �رջ��ѹ���
	RTC_WakeUpCmd(DISABLE);

	// ����ʱ��Դ��Ӳ��ʱ��Ƶ��Ϊ1Hz
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);

	// ֻ����һ�μ���
	RTC_SetWakeUpCounter(0);

	RTC_WakeUpCmd(ENABLE);

	// �����жϵĴ�����ʽ�������ж�
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	RTC_ClearFlag(RTC_FLAG_WUTF);

	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22; //
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // RTC�ֲ�涨
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//���¸�λ������ִ�еĺ�����������������ʱ��
void rtc_resume_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RTC_InitTypeDef RTC_InitStructure;
	// ʹ��rtc��Ӳ��ʱ��

	/* Enable the PWR clock��ʹ�ܵ�Դ����ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC���������RTC */
	PWR_BackupAccessCmd(ENABLE);

#if 0
	//��LSE��ʱ��
	RCC_LSEConfig(RCC_LSE_ON);

	//���LSE��ʱ���Ƿ����
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	
	//ΪRTCѡ��LSE��Ϊʱ��Դ
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#else

	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif

	/* Enable the RTC Clock��ʹ��RTC��Ӳ��ʱ�� */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation���ȴ�����RTC��ؼĴ������� */
	RTC_WaitForSynchro();

	// ����Ƶ��1Hz
	/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
	// 32768Hz/(127+1)/(255+1)=1Hz
	RTC_InitStructure.RTC_AsynchPrediv = 127;
	RTC_InitStructure.RTC_SynchPrediv = 255;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

	RTC_WakeUpCmd(DISABLE);

	// ����ʱ��Դ��Ӳ��ʱ��Ƶ��Ϊ1Hz
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);

	// ֻ����һ�μ���
	RTC_SetWakeUpCounter(0);

	RTC_WakeUpCmd(ENABLE);

	// �����жϵĴ�����ʽ�������ж�
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	RTC_ClearFlag(RTC_FLAG_WUTF);

	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22; //
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // RTC�ֲ�涨
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*�Լ���װ���ö�̬�������ӵĺ���*/
void rtc_alarm_init(uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t AlarmDateWeekDay)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmStructure;

	// �ر����ӹ���
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	/* Set the alarm 05h:20min:30s ��������*/
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = Hours;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = Minutes;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = Seconds;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = AlarmDateWeekDay;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_None;

	// ʹ�����ӹ���
	/* Configure the RTC Alarm A register ����ΪA����*/
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

	/* Enable RTC Alarm A Interrupt ʹ�������ж�*/
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);

	/* Enable the alarm ʹ������A*/
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	EXTI_InitStructure.EXTI_Line = EXTI_Line17;			   // ��ǰʹ���ⲿ�жϿ�����17
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	   // �ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // �����ش����ж�
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			   // ʹ���ⲿ�жϿ�����17
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;		 // ����RTC�����жϴ���
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x05; // ��ռ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;		 // ��Ӧ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // ʹ��
	NVIC_Init(&NVIC_InitStructure);
}

// ��int���͵�������0~59��ת��Ϊ8λ16����(���磺23->0x23)
uint8_t to_hex(uint32_t num)
{
	uint8_t hex_num = 0;

	hex_num = (num / 10) * 16 + (num % 10); // ��֧��2λʮ��������ת��

	return hex_num;
}

//ʵʱʱ�ӻ����ж�
void RTC_WKUP_IRQHandler(void)
{
	uint32_t ulReturn;

	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	// ����־λ
	if (RTC_GetITStatus(RTC_IT_WUT) == SET)
	{
		// �����¼���־��
		xEventGroupSetBitsFromISR(g_event_group, EVENT_GROUP_RTC_WAKEUP, NULL);

		// ��ձ�־λ
		RTC_ClearITPendingBit(RTC_IT_WUT);

		EXTI_ClearITPendingBit(EXTI_Line22);
	}

	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}

// ʵʱʱ��RTC�����ж�
void RTC_Alarm_IRQHandler(void)
{
	// ����־λ������A��
	if (RTC_GetITStatus(RTC_IT_ALRA) != RESET)
	{
		//���������¼�
		xEventGroupSetBitsFromISR(g_event_group, EVENT_GROUP_RTC_ALARM, NULL);
		
		// ��ձ�־λ
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);
	}
}

