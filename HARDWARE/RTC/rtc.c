#include "includes.h"

void rtc_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	RTC_InitTypeDef RTC_InitStructure;
	// 使能rtc的硬件时钟

	/* Enable the PWR clock，使能电源管理时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC，允许访问RTC */
	PWR_BackupAccessCmd(ENABLE);

#if 0
	//打开LSE振荡时钟
	RCC_LSEConfig(RCC_LSE_ON);

	//检查LSE振荡时钟是否就绪
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	
	//为RTC选择LSE作为时钟源
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#else
	/*使能LSI时钟（32KHz）*/
	RCC_LSICmd(ENABLE);
	/*等待LST时钟使能成功 Wait till LSI is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
		;
	/*选择时钟源LSI*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

#endif

	/*配置RTC输出频率1Hz-->设置预分频值*/
	/* RTC时钟计算方式ck_spre(1Hz) = RTCCLK(LSE 32Khz) /(uwAsynchPrediv + 1)/(uwSynchPrediv + 1)*/
	/* Enable the RTC Clock ，使能RTC时钟*/
	RCC_RTCCLKCmd(ENABLE);
	/* Wait for RTC APB registers synchronisation ，等待RTC相关寄存器就绪*/
	RTC_WaitForSynchro();

	/* Configure the RTC data register and RTC prescaler，配置RTC数据寄存器与RTC的分频值 */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;			  // 异步分频系数 128-1
	RTC_InitStructure.RTC_SynchPrediv = 0xF9;			  // 同步分频系数 250-1
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; // 24小时格式
	RTC_Init(&RTC_InitStructure);

	// 配置日期
	RTC_DateStructure.RTC_Year = 0x23;					   // 2023年
	RTC_DateStructure.RTC_Month = RTC_Month_November;	   // 11月份
	RTC_DateStructure.RTC_Date = 0x28;					   // 第28天/28日/28号
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Tuesday; // 星期2
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

	// 配置时间 17:14:50
	RTC_TimeStructure.RTC_H12 = RTC_H12_PM;
	RTC_TimeStructure.RTC_Hours = 0x11;
	RTC_TimeStructure.RTC_Minutes = 0x10;
	RTC_TimeStructure.RTC_Seconds = 0x50;
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

	// 关闭唤醒功能
	RTC_WakeUpCmd(DISABLE);

	// 唤醒时钟源的硬件时钟频率为1Hz
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);

	// 只进行一次计数
	RTC_SetWakeUpCounter(0);

	RTC_WakeUpCmd(ENABLE);

	// 配置中断的触发方式：唤醒中断
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	RTC_ClearFlag(RTC_FLAG_WUTF);

	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22; //
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // RTC手册规定
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// 优先级
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//按下复位按键后执行的函数，不会设置日期时间
void rtc_resume_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RTC_InitTypeDef RTC_InitStructure;
	// 使能rtc的硬件时钟

	/* Enable the PWR clock，使能电源管理时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC，允许访问RTC */
	PWR_BackupAccessCmd(ENABLE);

#if 0
	//打开LSE振荡时钟
	RCC_LSEConfig(RCC_LSE_ON);

	//检查LSE振荡时钟是否就绪
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
	
	//为RTC选择LSE作为时钟源
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

	/* Enable the RTC Clock，使能RTC的硬件时钟 */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation，等待所有RTC相关寄存器就绪 */
	RTC_WaitForSynchro();

	// 配置频率1Hz
	/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
	// 32768Hz/(127+1)/(255+1)=1Hz
	RTC_InitStructure.RTC_AsynchPrediv = 127;
	RTC_InitStructure.RTC_SynchPrediv = 255;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);

	RTC_WakeUpCmd(DISABLE);

	// 唤醒时钟源的硬件时钟频率为1Hz
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);

	// 只进行一次计数
	RTC_SetWakeUpCounter(0);

	RTC_WakeUpCmd(ENABLE);

	// 配置中断的触发方式：唤醒中断
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	RTC_ClearFlag(RTC_FLAG_WUTF);

	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22; //
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // RTC手册规定
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// 优先级
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*自己封装设置动态设置闹钟的函数*/
void rtc_alarm_init(uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t AlarmDateWeekDay)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmStructure;

	// 关闭闹钟功能
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	/* Set the alarm 05h:20min:30s 设置闹钟*/
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = Hours;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = Minutes;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = Seconds;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = AlarmDateWeekDay;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_None;

	// 使能闹钟功能
	/* Configure the RTC Alarm A register 配置为A闹钟*/
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

	/* Enable RTC Alarm A Interrupt 使能闹钟中断*/
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);

	/* Enable the alarm 使能闹钟A*/
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	EXTI_InitStructure.EXTI_Line = EXTI_Line17;			   // 当前使用外部中断控制线17
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	   // 中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; // 上升沿触发中断
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			   // 使能外部中断控制线17
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;		 // 允许RTC唤醒中断触发
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x05; // 抢占优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;		 // 响应优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能
	NVIC_Init(&NVIC_InitStructure);
}

// 将int类型的整数（0~59）转化为8位16进制(例如：23->0x23)
uint8_t to_hex(uint32_t num)
{
	uint8_t hex_num = 0;

	hex_num = (num / 10) * 16 + (num % 10); // 仅支持2位十进制整数转换

	return hex_num;
}

//实时时钟唤醒中断
void RTC_WKUP_IRQHandler(void)
{
	uint32_t ulReturn;

	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	// 检测标志位
	if (RTC_GetITStatus(RTC_IT_WUT) == SET)
	{
		// 设置事件标志组
		xEventGroupSetBitsFromISR(g_event_group, EVENT_GROUP_RTC_WAKEUP, NULL);

		// 清空标志位
		RTC_ClearITPendingBit(RTC_IT_WUT);

		EXTI_ClearITPendingBit(EXTI_Line22);
	}

	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}

// 实时时钟RTC闹钟中断
void RTC_Alarm_IRQHandler(void)
{
	// 检测标志位（闹钟A）
	if (RTC_GetITStatus(RTC_IT_ALRA) != RESET)
	{
		//触发闹钟事件
		xEventGroupSetBitsFromISR(g_event_group, EVENT_GROUP_RTC_ALARM, NULL);
		
		// 清空标志位
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);
	}
}

