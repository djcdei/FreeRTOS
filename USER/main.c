
#include "includes.h"

/*任务句柄，一个硬件一个任务句柄控制*/
static TaskHandle_t app_task_init_handle = NULL;
static TaskHandle_t app_task_led_handle = NULL;
static TaskHandle_t app_task_led_breath_handle = NULL;
static TaskHandle_t app_task_beep_handle = NULL;
static TaskHandle_t app_task_key_handle = NULL;
static TaskHandle_t app_task_keyboard_handle = NULL;
static TaskHandle_t app_task_rtc_handle = NULL;
static TaskHandle_t app_task_flash_handle = NULL;
static TaskHandle_t app_task_dht_handle = NULL;
static TaskHandle_t app_task_sr04_handle = NULL;
static TaskHandle_t app_task_oled_handle = NULL;
static TaskHandle_t app_task_usart_handle = NULL;
static TaskHandle_t app_task_mfrc522real_handle = NULL;
static TaskHandle_t app_task_mfrc522admin_handle = NULL;
static TaskHandle_t app_task_password_man_handle = NULL;
static TaskHandle_t app_task_fpm383_handle = NULL;

/*任务函数入口*/
static void app_task_init(void *pvParameters);		   /* 	设备初始化任务 */
static void app_task_led(void *pvParameters);		   /* 	led任务 接收LED消息队列的信息并做出反应*/
static void app_task_led_breath(void *pvParameters);   /*	呼吸灯任务，存放解锁记录等信息*/
static void app_task_beep(void *pvParameters);		   /* 	beep任务 接收蜂鸣器消息队列的信息并做出相应反应*/
static void app_task_key(void *pvParameters);		   /* 	key任务 */
static void app_task_keyboard(void *pvParameters);	   /* 	keyboard任务 */
static void app_task_rtc(void *pvParameters);		   /*	rtc实时时钟任务*/
static void app_task_dht(void *pvParameters);		   /* 	dth11任务 获取温湿度信息，发送到oled显示屏消息队列*/
static void app_task_sr04(void *pvParameters);		   /*	sr04任务，获取距离信息，发送到oled显示屏消息队列*/
static void app_task_oled(void *pvParameters);		   /*	oled任务，接收并解析g_queue_oled队列各种信息，显示出来*/
static void app_task_flash(void *pvParameters);		   /*	flash任务，存放解锁记录等信息*/
static void app_task_usart(void *pvParameters);		   /*	usart任务，接收串口队列的信息并解析*/
static void app_task_mfrc522real(void *pvParameters);  /*	rfid读卡任务函数*/
static void app_task_mfrc522admin(void *pvParameters); /*	rfid管理任务函数*/
static void app_task_password_man(void *pvParameters); /*	password management任务，接收矩阵键盘队列的信息并解析*/
static void app_task_fpm383(void *pvParameters);	   /*	fpm383电容指纹任务函数*/
/*计数型信号量句柄*/
SemaphoreHandle_t g_sem_led;  // 同步led任务和其他任务
SemaphoreHandle_t g_sem_beep; // 同步beep任务和其他任务

/*互斥型信号量句柄*/
SemaphoreHandle_t g_mutex_oled;

/*事件标志组句柄（32bit位标志组）便于管理多个任务标志位*/
EventGroupHandle_t g_event_group;

/*消息队列句柄*/
QueueHandle_t g_queue_usart; // 接收串口发送来的消息
QueueHandle_t g_queue_led;
QueueHandle_t g_queue_beep;
QueueHandle_t g_queue_oled;
QueueHandle_t g_queue_flash;
QueueHandle_t g_queue_keyboard;

/*全局变量*/
volatile uint32_t g_rtc_get_what = FLAG_RTC_GET_NONE;
volatile uint32_t g_dht_get_what = FLAG_DHT_GET_NONE;
volatile uint32_t g_unlock_what = FLAG_UNLOCK_NO;
volatile uint32_t g_pass_man_what = FLAG_PASS_MAN_NONE;					 // 密码管理标志
volatile uint32_t g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD;			 // 密码解锁模式。默认键盘解锁，根据不同解锁方式会自动更改该标志位
const char pass_auth_default[PASS_LEN] = {'8', '8', '8', '8', '8', '8'}; // 默认密码

int main(void)
{
	/* 设置系统中断优先级分组只能选择第4组 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 系统定时器中断频率为configTICK_RATE_HZ 168000000/1000 */
	SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);

	//	/* 初始化串口1 */
	//	usart1_init(115200);
	/*初始化串口3*/
	usart3_init(9600);

	/* 创建 app_task_init任务 */
	xTaskCreate((TaskFunction_t)app_task_init,			/* 任务入口函数 */
				(const char *)"app_task_init",			/* 任务名字 */
				(uint16_t)512,							/* 任务栈大小 */
				(void *)NULL,							/* 任务入口函数参数 */
				(UBaseType_t)5,							/* 任务的优先级 */
				(TaskHandle_t *)&app_task_init_handle); /* 任务控制块指针 */

	/* 开启任务调度 */
	vTaskStartScheduler();

	while (1)
		;
}

/* 任务列表 */
static const task_t task_tbl[] = {
	{app_task_usart, "app_task_usart", 1024, NULL, 5, &app_task_usart_handle},
	{app_task_flash, "app_task_flash", 512, NULL, 5, &app_task_flash_handle},
	{app_task_key, "app_task_key", 512, NULL, 5, &app_task_key_handle},
	{app_task_keyboard, "app_task_keyboard", 512, NULL, 5, &app_task_keyboard_handle},
	{app_task_dht, "app_task_dht", 512, NULL, 5, &app_task_dht_handle},
	{app_task_rtc, "app_task_rtc", 512, NULL, 5, &app_task_rtc_handle},
	{app_task_led, "app_task_led", 512, NULL, 5, &app_task_led_handle},
	{app_task_led_breath, "app_task_led_breath", 512, NULL, 5, &app_task_led_breath_handle},
	{app_task_beep, "app_task_beep", 512, NULL, 5, &app_task_beep_handle},
	{app_task_sr04, "app_task_sr04", 512, NULL, 5, &app_task_sr04_handle},
	{app_task_oled, "app_task_oled", 512, NULL, 5, &app_task_oled_handle},
	{app_task_password_man, "app_task_password_man", 512, NULL, 5, &app_task_password_man_handle},
	{app_task_fpm383, "app_task_fpm383", 512, NULL, 5, &app_task_fpm383_handle},
	{0, 0, 0, 0, 0, 0}};

static void app_task_init(void *pvParameters)
{
	uint32_t i = 0;
	printf("app_task_init is running ...\r\n");

	/* 创建用到的任务 */
	taskDISABLE_INTERRUPTS();
	i = 0;
	while (task_tbl[i].pxTaskCode)
	{
		xTaskCreate(task_tbl[i].pxTaskCode,		/* 任务入口函数 */
					task_tbl[i].pcName,			/* 任务名字 */
					task_tbl[i].usStackDepth,	/* 任务栈大小 */
					task_tbl[i].pvParameters,	/* 任务入口函数参数 */
					task_tbl[i].uxPriority,		/* 任务的优先级 */
					task_tbl[i].pxCreatedTask); /* 任务控制块指针 */
		i++;
	}
	taskENABLE_INTERRUPTS();

	/*创建计数型信号量，最大值为10，初值为0*/
	g_sem_led = xSemaphoreCreateCounting(10, 0);
	g_sem_beep = xSemaphoreCreateCounting(10, 0);
	/*创建事件标志组*/
	g_event_group = xEventGroupCreate();
	/*创建消息队列,要注意消息队列的容量是否创建正确*/
	g_queue_led = xQueueCreate(QUEUE_LED_LEN, sizeof(uint8_t));			  // 用于led通信的消息队列
	g_queue_beep = xQueueCreate(QUEUE_BEEP_LEN, sizeof(beep_t));		  // 用于beep通信的消息队列
	g_queue_oled = xQueueCreate(QUEUE_OLED_LEN, sizeof(oled_t));		  // 用于oled通信的消息队列
	g_queue_usart = xQueueCreate(QUEUE_USART_LEN, sizeof(usart_t));		  // 用于usart通信的消息队列
	g_queue_flash = xQueueCreate(QUEUE_FLASH_LEN, sizeof(flash_t));		  // 用于flash通信的消息队列
	g_queue_keyboard = xQueueCreate(QUEUE_KEYBOARD_LEN, sizeof(uint8_t)); // 用于keyboard通信的消息队列

	/*实现了复位不重置flash和实时时钟当前日期时间*/
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 4455)
	{
		rtc_init();	  // 实时时钟RTC初始化，并设置日期和时间
		flash_init(); // 清空扇区4
		RTC_WriteBackupRegister(RTC_BKP_DR0, 4455);
	}
	else
	{
		rtc_resume_init(); // 实时时钟RTC初始化，不设置日期和时间
	}

	led_init();
	led_breath_init();
	beep_init();
	sr04_init();
	dht11_init();
	key_init();
	kbd_init();
	OLED_Init();
	fpm_init();
	OLED_Clear();
	/* 显示logo */
	OLED_DrawBMP(0, 0, 128, 8, (uint8_t *)pic_logo);
	vTaskDelay(3000);
	OLED_Clear();
	OLED_DrawBMP(32, 0, 96, 8, (uint8_t *)pic_lock_icon);

	/* 删除任务自身 */
	vTaskDelete(NULL);
	printf("[app_task_init] nerver run here\r\n");
}

/*led任务函数*/
static void app_task_led(void *pvParameters)
{
	uint8_t led_sta = 0; // led状态标志位
	BaseType_t xReturn = pdFALSE;
	printf("app_task_led is running ...\r\n");
	for (;;) // while(1)
	{
		xReturn = xQueueReceive(g_queue_led,	/* 消息队列的句柄 */
								&led_sta,		/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		// 解析消息队列传递来的信息
		if (xReturn != pdPASS)
			continue;
		/* 检测到控制LED1 亮：00010001(0x11)，灭：00010000(0x10)	*/
		if (led_sta & 0x10) // 巧妙之处在于通过按位与实现利用一个字节(8bit)来控制led灯的亮灭：前4bit选择灯，最后1bit控制亮灭
		{
			if (led_sta & 0x01)
				PFout(9) = 0;
			else
				PFout(9) = 1;
		}
		/* 检测到控制LED2 亮：00100001(0x21)，灭：00100000(0x20)	*/
		if (led_sta & 0x20)
		{
			if (led_sta & 0x01)
				PFout(10) = 0;
			else
				PFout(10) = 1;
		}
		/* 检测到控制LED3 01000001(0x41)，灭：01000000(0x40)	*/
		if (led_sta & 0x40)
		{
			if (led_sta & 0x01)
				PEout(13) = 0;
			else
				PEout(13) = 1;
		}
		/* 检测到控制LED4 10000001(0x81)，灭：10000000(0x80)	*/
		if (led_sta & 0x80)
		{
			if (led_sta & 0x01)
				PEout(14) = 0;
			else
				PEout(14) = 1;
		}
		/* 释放信号量，告诉对方，当前led控制任务已经完成 */
		xSemaphoreGive(g_sem_led);
	}
}
/*呼吸灯模块*/
void app_task_led_breath(void *pvParameters)
{
	int8_t brightness = 0;

	printf("[app_task_led_breath] create success and suspend self\r\n");

	vTaskSuspend(NULL); // 先挂起呼吸灯

	printf("[app_task_led_breath] resume success\r\n");

	for (;;)
	{
		/* 渐亮 */
		for (brightness = 0; brightness <= 100; brightness++)
		{
			led_breath_brightness(brightness);
			vTaskDelay(20);
		}

		/* 渐灭 */
		for (brightness = 100; brightness >= 0; brightness--)
		{
			led_breath_brightness(brightness);
			vTaskDelay(20);
		}
	}
}
/*蜂鸣器模块*/
static void app_task_beep(void *pvParameters)
{
	beep_t beep;
	BaseType_t xReturn;
	printf("app_task_beep is running ...\r\n");
	for (;;) // while(1)
	{
		xReturn = xQueueReceive(g_queue_beep,	/* 消息队列的句柄 */
								&beep,			/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		// 解析消息队列传递来的信息
		if (xReturn != pdPASS)
			continue;
		// printf("[app_task_beep] beep.sta=%d beep.duration=%d\r\n", beep.sta, beep.duration);
		// 检查蜂鸣器是否需要持续工作
		if (beep.duration)
		{
			BEEP(beep.sta);
			while (beep.duration--)
				vTaskDelay(1);
			/* 蜂鸣器状态翻转 */
			beep.sta ^= 1;
		}
		BEEP(beep.sta);
		/* 释放信号量，告诉对方，当前beep控制任务已经完成 */
		xSemaphoreGive(g_sem_beep);
	}
}
/*按键模块*/
static void app_task_key(void *pvParameters)
{
	uint8_t buf[16] = 0;
	beep_t beep;
	oled_t oled;
	EventBits_t EventValue;
	BaseType_t xReturn = pdFALSE;
	printf("[app_task_key] create success\r\n");
	for (;;)
	{
		// 等待事件组中的相应事件位，或同步
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_KEYALL_DOWN,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
		memset(&oled, 0, sizeof(oled));
		// 根据返回值判断哪个按键触发了
		if (EventValue & EVENT_GROUP_KEY1_DOWN) // 按键1
		{
			// 禁止EXTI0触发中断
			NVIC_DisableIRQ(EXTI0_IRQn);
			// 延时消抖
			vTaskDelay(50);
			// 确认是按下
			if (PAin(0) == 0)
			{
				printf("[app_task_key] S1 Press\r\n");
				vTaskSuspend(app_task_rtc_handle);	// 挂起实时时钟任务
				vTaskSuspend(app_task_sr04_handle); // 挂起超声波模块
				g_rtc_get_what = FLAG_RTC_GET_NONE; // 不显示时间信息
				flash_read(10);
				// 发送注册指纹事件
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_ADD);
			}
			// 等待按键释放
			while (PAin(0) == 0)
				vTaskDelay(1);
			// 允许EXTI0触发中断
			NVIC_EnableIRQ(EXTI0_IRQn);
		}
		if (EventValue & EVENT_GROUP_KEY2_DOWN) // 按键2
		{
			// 禁止EXTI2触发中断
			NVIC_DisableIRQ(EXTI2_IRQn);
			// 延时消抖
			vTaskDelay(50);
			if (PEin(2) == 0)
			{
				printf("[app_task_key] S2 Press\r\n");
				vTaskSuspend(app_task_rtc_handle);	// 挂起实时时钟任务
				vTaskSuspend(app_task_sr04_handle); // 挂起超声波模块
				g_rtc_get_what = FLAG_RTC_GET_NONE; // 	不显示时间信息

				// 	// 触发事件标志位0x20,触发温度记录任务
				// xEventGroupSetBits(g_event_group, EVENT_GROUP_DHT);
				// 发送验证指纹事件
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_AUTH);
			}
			// 等待按键释放
			while (PEin(2) == 0)
				vTaskDelay(1);
			// 允许EXTI2触发中断
			NVIC_EnableIRQ(EXTI2_IRQn);
		}
		if (EventValue & EVENT_GROUP_KEY3_DOWN) // 按键3
		{
			// 禁止EXTI3触发中断
			NVIC_DisableIRQ(EXTI3_IRQn);
			// 延时消抖
			vTaskDelay(50);
			if (PEin(3) == 0)
			{
				printf("[app_task_key] S1 Press\r\n");
				g_rtc_get_what = FLAG_RTC_GET_NONE;
				// 发送显示指纹数量事件
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_SHOW);
			}
			// 等待按键释放
			while (PEin(3) == 0)
				vTaskDelay(1);
			// 允许EXTI3触发中断
			NVIC_EnableIRQ(EXTI3_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY4_DOWN) // 按键4
		{
			// 禁止EXTI4触发中断
			NVIC_DisableIRQ(EXTI4_IRQn);
			// 延时消抖
			vTaskDelay(50);
			if (PEin(4) == 0)
			{
				printf("[app_task_key] S4 Press\r\n");
				// 发送删除指纹事件
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_DELETE);
				vTaskResume(app_task_rtc_handle);	// 恢复rtc任务
				g_rtc_get_what = FLAG_RTC_GET_TIME; // 获取时间
				// g_rtc_get_what = FLAG_RTC_GET_DATE; // 获取日期

				oled.ctrl = OLED_CTRL_CLEAR; /* oled清屏 */
				xReturn = xQueueSend(g_queue_oled, &oled, 100);
				if (xReturn != pdPASS)
					printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			}

			// 等待按键释放
			while (PEin(4) == 0)
				vTaskDelay(1);

			NVIC_EnableIRQ(EXTI4_IRQn); // 允许EXTI4触发中断
		}
	}
}
/*矩阵键盘模块*/
void app_task_keyboard(void *pvParameters)
{
	char key_val = 'N';
	beep_t beep;
	BaseType_t xReturn;
	printf("[app_task_kbd] create success\r\n");

	while (1)
	{
		/* 读取矩阵键盘按键值 */
		key_val = kbd_read();

		if (key_val != 'N')
		{
			printf("[app_task_kbd] kbd press %c \r\n", key_val);

			xReturn = xQueueSend(g_queue_keyboard, &key_val, 100);
			if (xReturn != pdPASS)
				printf("[app_task_kbd] xQueueSend kbd error code is %d\r\n", xReturn);

			/* 嘀一声示意 */
			beep.sta = 1;
			beep.duration = 10;
			xReturn = xQueueSend(g_queue_beep, &beep, 100);
			if (xReturn != pdPASS)
				printf("[app_task_kbd] xQueueSend beep error code is %d\r\n", xReturn);

			/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				printf("[app_task_system_reset] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
		}
	}
}
/*rtc实时时钟模块*/
static void app_task_rtc(void *pvParameters)
{
	uint8_t buf[16] = {0};
	oled_t oled;
	BaseType_t xReturn;
	EventBits_t EventValue;

	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	printf("[app_task_rtc] create success and suspend self\r\n");
	vTaskSuspend(app_task_rtc_handle);
	printf("[app_task_rtc] resume success\r\n");

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_RTC_WAKEUP | EVENT_GROUP_RTC_ALARM,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		if (EventValue & EVENT_GROUP_RTC_WAKEUP) // RTC事件被触发
		{
			memset(buf, 0, sizeof buf);
			if (g_rtc_get_what == FLAG_RTC_GET_TIME)
			{
				/* RTC_GetTime，获取时间 */
				RTC_GetTime(RTC_Format_BCD, &RTC_TimeStructure);
				memset(buf, 0, sizeof buf);
				/* 格式化字符串 */
				sprintf((char *)buf, "%02x:%02x:%02x", RTC_TimeStructure.RTC_Hours,
						RTC_TimeStructure.RTC_Minutes,
						RTC_TimeStructure.RTC_Seconds);

				oled.x = 32;
			}
			if (g_rtc_get_what == FLAG_RTC_GET_DATE)
			{
				/* RTC_GetTime，获取日期 */
				RTC_GetDate(RTC_Format_BCD, &RTC_DateStructure);

				memset(buf, 0, sizeof buf);
				/* 格式化字符串 */
				sprintf((char *)buf, "20%02x-%02x-%02x-%d", RTC_DateStructure.RTC_Year,
						RTC_DateStructure.RTC_Month,
						RTC_DateStructure.RTC_Date,
						RTC_DateStructure.RTC_WeekDay);
				oled.x = 15;
			}
			/* oled显示时间 */
			oled.ctrl = OLED_CTRL_SHOW_STRING;
			oled.y = 4;
			oled.str = buf; // 字符串缓冲区首地址
			oled.font_size = 16;

			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
		}
		if (EventValue & EVENT_GROUP_RTC_ALARM) // 闹钟被唤醒事件
		{
			printf("闹钟唤醒了\r\n");
		}
	}
}
/*温湿度模块*/
static void app_task_dht(void *pvParameters)
{
	static uint32_t cnt = 0; // 限制记录的温度只有10条
	uint8_t dht11_buf[5];	 // 存储温湿度模块读取到的数据
	int32_t dht11_read_ret;	 // 温湿度读取函数返回值
	uint8_t g_temp_buf[32];	 // 格式化记录当前温湿度
	oled_t oled;
	flash_t flash;
	BaseType_t xReturn;
	EventBits_t EventValue;

	for (;;) // while(1)
	{
		// 等待事件组中的相应事件位，或同步
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_DHT,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
		if (EventValue & EVENT_GROUP_DHT)
		{
			printf("app_task_dht is running ...\r\n");
			dht11_read_ret = dht11_read(dht11_buf);
			if (dht11_read_ret == 0)
			{
				memset(&oled, 0, sizeof(oled));
				memset(&flash, 0, sizeof(flash));
				// 将数据封装成flash存储结构体格式，发送到flash队列
				flash.mode = MODE_OPEN_LOCK_DHT;
				sprintf((char *)flash.databuf, "Temp=%d.%d Humi=%d.%d", dht11_buf[2], dht11_buf[3], dht11_buf[0], dht11_buf[1]);
				xReturn = xQueueSend(g_queue_flash, /* 消息队列的句柄 */
									 &flash,		/* 发送的消息内容 */
									 100);			/* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_dht] xQueueSend flash string error code is %d\r\n", xReturn);

				// 将数据封装成oled格式发送给oled队列
				sprintf((char *)g_temp_buf, "Temp=%d.%d", dht11_buf[2], dht11_buf[3]);
				// 发送给g_queue_oled
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.x = 32;
				oled.y = 2;
				oled.str = g_temp_buf; // 字符串缓冲区首地址
				oled.font_size = 16;
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
				vTaskDelay(500); // 不加延时后面的数据会覆盖前面的
				memset(&oled, 0, sizeof(oled));
				sprintf((char *)g_temp_buf, "Humi=%d.%d", dht11_buf[0], dht11_buf[1]);
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.x = 32;
				oled.y = 6;
				oled.str = g_temp_buf; // 字符串缓冲区首地址
				oled.font_size = 16;
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
			}
			else
			{
				printf("dht11 read error code is %d\r\n", dht11_read_ret);
			}
		}
		vTaskResume(app_task_sr04_handle); // 恢复超声波模块
	}
}
/*超声波模块*/
static void app_task_sr04(void *pvParameters)
{
	int distance_save;
	oled_t oled;
	EventBits_t xReturn;
	for (;;) // while(1)
	{
		printf("app_task_sr04 is running ...\r\n");
		distance_save = sr04_get_distance();
		switch (distance_save)
		{
		case 0:
			printf("distance error\r\n");
			break;
		case 1:
			oled.ctrl = OLED_CTRL_DISPLAY_OFF;
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			break;
		case 2:
			// 息屏
			oled.ctrl = OLED_CTRL_DISPLAY_OFF;
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			break;
		case 3:
			// 亮屏
			oled.ctrl = OLED_CTRL_DISPLAY_ON;
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			break;
		default:
			break;
		}
		vTaskDelay(3000);
	}
}
/*oled模块*/
static void app_task_oled(void *pvParameters)
{
	oled_t oled;
	BaseType_t xReturn = pdFALSE;
	printf("[app_task_oled] create success\r\n");
	memset(&oled, 0, sizeof(oled));
	for (;;)
	{
		xReturn = xQueueReceive(g_queue_oled,	/* 消息队列的句柄 */
								&oled,			/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		if (xReturn != pdPASS)
			continue;

		switch (oled.ctrl)
		{
		case OLED_CTRL_DISPLAY_ON:
		{
			/* 亮屏 */
			OLED_Display_On();
			/* 呼吸灯停止工作 */
			vTaskSuspend(app_task_led_breath_handle);
			led_breath_stop();
		}
		break;

		case OLED_CTRL_DISPLAY_OFF:
		{
			/* 灭屏 */
			OLED_Display_Off();
			/* 呼吸灯开始工作 */
			led_breath_run();
			vTaskResume(app_task_led_breath_handle);
		}
		break;

		case OLED_CTRL_CLEAR:
		{
			/* 清屏 */
			OLED_Clear();
		}
		break;

		case OLED_CTRL_SHOW_STRING:
		{
			/* 显示字符串 */
			OLED_ShowString(oled.x,
							oled.y,
							oled.str,
							oled.font_size);
		}
		break;

		case OLED_CTRL_SHOW_CHINESE:
		{
			/* 显示汉字 */
			OLED_ShowCHinese(oled.x,
							 oled.y,
							 oled.chinese);
		}
		break;

		case OLED_CTRL_SHOW_PICTURE:
		{
			/* 显示图片 */
			OLED_DrawBMP(oled.x,
						 oled.y,
						 oled.x + oled.pic_width,
						 oled.y + oled.pic_height,
						 oled.pic);
		}
		break;

		default:
			printf("[app_task_oled] oled ctrl code is invalid\r\n");
			break;
		}
	}
}

static void app_task_flash(void *pvParameters)
{
	flash_t flash_write_buf;
	BaseType_t xReturn = pdFALSE;
	RTC_DateTypeDef RTC_Date;
	RTC_TimeTypeDef RTC_Time;
	printf("[app_task_flash] create success\r\n");
	for (;;)
	{
		xReturn = xQueueReceive(g_queue_flash,	  /* 消息队列的句柄 */
								&flash_write_buf, /* 接收的消息内容 */
								portMAX_DELAY);	  /* 等待时间一直等 */
		if (xReturn != pdPASS)
			continue;

		printf("[app_task_flash] wait get data\r\n");

		// 获取当前扇区写入块标识
		flash_write_buf.offset = flash_read_offset((uint32_t *)(0x08010000));
		if (flash_write_buf.offset >= 10)
		{
			flash_init();				// 清空数据
			flash_write_buf.offset = 0; // 从0开始重新写入
		}

		RTC_GetDate(RTC_Format_BCD, &RTC_Date);
		RTC_GetTime(RTC_Format_BCD, &RTC_Time);

		printf("当前时间:20%02x/%02x/%02x %02x:%02x:%02x %d\n",
			   RTC_Date.RTC_Year,
			   RTC_Date.RTC_Month,
			   RTC_Date.RTC_Date,
			   RTC_Time.RTC_Hours,
			   RTC_Time.RTC_Minutes,
			   RTC_Time.RTC_Seconds,
			   flash_write_buf.mode);

		// 记录下每次写入数据的日期时间
		flash_write_buf.date[0] = RTC_Date.RTC_Year;
		flash_write_buf.date[1] = RTC_Date.RTC_Month;
		flash_write_buf.date[2] = RTC_Date.RTC_Date;
		flash_write_buf.date[3] = RTC_Time.RTC_Hours;
		flash_write_buf.date[4] = RTC_Time.RTC_Minutes;
		flash_write_buf.date[5] = RTC_Time.RTC_Seconds;

		taskENTER_CRITICAL();
		flash_write(&flash_write_buf);
		taskEXIT_CRITICAL();
	}
}

/*串口模块*/
static void app_task_usart(void *pvParameters)
{
	uint8_t years;	 // 年
	uint8_t month;	 // 月
	uint8_t week;	 // 周
	uint8_t date;	 // 闹钟：日
	uint8_t hours;	 // 闹钟：时
	uint8_t minutes; // 闹钟：分
	uint8_t seconds; // 闹钟：秒
	usart_t usart_packet;

	uint32_t i = 0;
	char *p = NULL;
	char key_val = 0;

	BaseType_t xReturn = pdFALSE;

	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	printf("[app_task_usart] create success\r\n");
	for (;;)
	{
		// 消息队列为空的时候会阻塞，不需要用事件标志组触发任务
		xReturn = xQueueReceive(g_queue_usart,	/* 消息队列的句柄 */
								&usart_packet,	/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		if (xReturn != pdPASS)
		{
			printf("[app_task_usart] xQueueReceive usart_packet error code is %d\r\n", xReturn);
			continue;
		}
		// 解析消息
		printf("[app_task_usart] recv data:%s\r\n", usart_packet.rx_buf);

		// 启动温湿度数据记录模式
		if (strstr((char *)usart_packet.rx_buf, "temp?#") || strstr((char *)usart_packet.rx_buf, "start"))
		{
		}
		if (strstr((char *)usart_packet.rx_buf, "ALARM SET")) // 设置闹钟：日-时-分-秒
		{
			// 截取 日-时-分-秒
			p = strtok((char *)usart_packet.rx_buf, "-");
			// printf("p:%s\r\n",p);
			date = to_hex(atoi(strtok(NULL, "-")));
			// printf("date:%d\r\n",date);
			hours = to_hex(atoi(strtok(NULL, "-")));
			// printf("hours:%d\r\n",hours);
			minutes = to_hex(atoi(strtok(NULL, "-")));
			// printf("minutes:%d\r\n",minutes);
			seconds = to_hex(atoi(strtok(NULL, "#")));
			// printf("seconds:%d\r\n",seconds);
			rtc_alarm_init(hours, minutes, seconds, date); // 设置
			printf("[app_task_usart] rtc set alarm ok\r\n");
		}

		/* 设置日期 */
		if (strstr((char *)usart_packet.rx_buf, "DATE SET"))
		{
			/* 以等号分割字符串 */
			p = strtok((char *)usart_packet.rx_buf, "-");
			// 获取年
			// 2021-2000=21
			years = to_hex(atoi(strtok(NULL, "-")) - 2000);
			RTC_DateStructure.RTC_Year = years;
			// 获取月
			month = to_hex(atoi(strtok(NULL, "-")));
			RTC_DateStructure.RTC_Month = month;
			// 获取日
			date = to_hex(atoi(strtok(NULL, "-")));
			RTC_DateStructure.RTC_Date = date;
			// 获取星期
			week = to_hex(atoi(strtok(NULL, "-")));
			RTC_DateStructure.RTC_WeekDay = week;
			// 设置
			RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
			printf("[app_task_usart] rtc set date ok\r\n");
		}
		/* 设置时间 */
		else if (strstr((char *)usart_packet.rx_buf, "TIME SET"))
		{
			/* 以等号分割字符串 */
			p = strtok((char *)usart_packet.rx_buf, "-");
			/* 获取时 */
			p = strtok(NULL, "-");
			i = atoi(p);
			/* 通过时，判断是AM还是PM */
			if (i < 12)
				RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
			else
				RTC_TimeStructure.RTC_H12 = RTC_H12_PM;

			/* 转换为BCD编码 */
			i = (i / 10) * 16 + i % 10;
			RTC_TimeStructure.RTC_Hours = i;
			/* 获取分 */
			minutes = to_hex(atoi(strtok(NULL, "-")));
			RTC_TimeStructure.RTC_Minutes = minutes;
			/* 获取秒 */
			seconds = to_hex(atoi(strtok(NULL, "#")));
			RTC_TimeStructure.RTC_Seconds = seconds;

			/* 设置RTC时间 */
			RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

			printf("[app_task_usart] rtc set time ok\r\n");
		}
		else if (strstr((char *)usart_packet.rx_buf, "PASS UNLOCK") && (g_pass_man_what == FLAG_PASS_MAN_AUTH)) // 验证密码
		{
			p = strtok((char *)usart_packet.rx_buf, "PASS UNLOCK");
			printf("[app_task_usart] recv pass :%s\r\n", p);
			g_pass_unlock_mode = MODE_OPEN_LOCK_BLUE; // 蓝牙解锁模式
			for (i = 0; i < 8; i++)
			{
				key_val = p[i];
				xReturn = xQueueSend(g_queue_keyboard, /* 消息队列的句柄 */
									 &key_val,		   /* 发送的消息内容 */
									 100);			   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_kbd] xQueueSend kbd error code is %d\r\n", xReturn);
			}
		}
	}
}

/*密码管理模块*/
void app_task_password_man(void *pvParameters)
{
	char pass_auth_eeprom[PASS_LEN] = {0}; // 存储在e2prom里面的密码

	char key_val = 0;
	char key_buf[PASS_LEN] = {0};

	uint32_t key_cnt = 0;
	int32_t rt;
	const uint8_t x_start = 36;
	const uint8_t y_start = 4;
	uint8_t x = x_start, y = y_start;

	oled_t oled;
	beep_t beep;
	flash_t flash;
	BaseType_t xReturn;

	for (;;)
	{
		// 等待矩阵键盘消息队列消息
		xReturn = xQueueReceive(g_queue_keyboard, &key_val, portMAX_DELAY);
		if (xReturn != pdPASS)
			continue;

		if (key_val == 'C')
		{
			printf("password reset\r\n");
			g_unlock_what = FLAG_UNLOCK_NO;		  // 设备上锁
			g_pass_man_what = FLAG_PASS_MAN_AUTH; // 开始密码验证
			/*恢复初值*/
			x = x_start;
			y = y_start;
			key_cnt = 0;
			memset(key_buf, 0, sizeof key_buf);
			// 显示密码输入界面
			oled.ctrl = OLED_CTRL_CLEAR;
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
			oled.ctrl = OLED_CTRL_SHOW_STRING;
			oled.x = x;
			oled.y = y;
			oled.font_size = 16;
			oled.str = "------";
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

			continue;
		}
		if (g_pass_man_what == FLAG_PASS_MAN_AUTH) // 开启密码验证
		{
			/* 没有密码，无效删除操作 */
			if (key_val == '*' && key_cnt == 0)
				continue;

			/* 删除上一格密码 */
			if (key_val == '*' && key_cnt)
			{
				key_cnt--;
				key_buf[key_cnt] = 0;
				x -= 8;
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.x = x;
				oled.y = y;
				oled.font_size = 16;
				oled.str = "-";
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
				continue;
			}

			if (key_cnt < 6)
			{
				/* 显示数字 */
				key_buf[key_cnt] = key_val;
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.x = x;
				oled.y = y;
				oled.font_size = 16;
				oled.str = (uint8_t *)&key_buf[key_cnt];
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

				vTaskDelay(100);
				/* 显示* */
				oled.str = "*";
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

				x += 8;
				key_cnt++;
			}
			else if (key_val == '#' && key_cnt >= 6)
			{
				printf("[app_task_pass_man] auth key buf is %6s\r\n", key_buf);

				/* 读取eeprom存储的密码 */
				// at24c02_read(PASS1_ADDR_IN_EEPROM, (uint8_t *)pass_auth_eeprom, PASS_LEN);

				/* 匹配默认密码 */
				rt = memcmp(pass_auth_default, key_buf, PASS_LEN);
				/* 密码匹配成功 */
				if (rt == 0)
				{
					printf("[app_task_pass_man] password auth success\r\n");
					g_unlock_what = FLAG_UNLOCK_OK;		  // 设备解锁
					g_pass_man_what = FLAG_PASS_MAN_NONE; // 不再进行密码验证
					/* 设置要显示的图片-表情（成功） */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_unlock_icon;

					xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
										 &oled,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* 嘀一声示意:第一次 */
					beep.sta = 1;
					beep.duration = 20;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					vTaskDelay(100);
					/* 嘀一声示意:第二次 */
					beep.sta = 1;
					beep.duration = 20;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					/* 延时一会 */
					vTaskDelay(2000);
					/* 恢复初值 */
					x = x_start;
					key_cnt = 0;
					memset(key_buf, 0, sizeof key_buf);
					/*显示菜单界面*/
					// 待补充...
					//  发送flash消息记录解锁日志
					flash.mode = g_pass_unlock_mode; // 获取当前解锁模式，有蓝牙解锁、键盘解锁，存入flash
					xReturn = xQueueSend(g_queue_flash, &flash, 100);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend flash error code is %d\r\n", xReturn);
				}
				else
				{
					printf("[app_task_pass_man] password auth fail\r\n");
					g_pass_man_what = FLAG_PASS_MAN_NONE; // 回到锁屏界面，等待下一次解锁
					/* 设置要显示的图片-表情（失败） */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_error_icon;

					xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
										 &oled,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* 长鸣1秒示意 */
					beep.sta = 1;
					beep.duration = 1000;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);

					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);

					/* 延时一会 */
					vTaskDelay(2000);
					/* 恢复初值 */
					x = x_start;
					key_cnt = 0;
					memset(key_buf, 0, sizeof key_buf);
					/* 显示锁屏界面 */
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.pic = pic_lock_icon;
					xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
										 &oled,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
				}
				g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // 恢复默认解锁方式
			}
		}
	}
}
/*指纹识别模块*/
void app_task_fpm383(void *pvParameters)
{
	uint8_t fmp_error_code;
	uint16_t id;
	uint16_t id_total;
	beep_t beep;
	oled_t oled;
	flash_t flash;
	EventBits_t EventValue;
	BaseType_t xReturn = pdFALSE;
	printf("[app_task_fpm383] create success\r\n");
	for (;;)
	{
		// 等待事件组中的相应事件位，或同步
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_FPM_ADD | EVENT_GROUP_FPM_AUTH | EVENT_GROUP_FPM_DELETE | EVENT_GROUP_FPM_SHOW,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
										 
		if (g_unlock_what == FLAG_UNLOCK_NO) // 设备上锁后才能解锁
		{
			if (EventValue & EVENT_GROUP_FPM_AUTH) // 验证指纹事件
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				printf("[app_task_fpm383] 执行刷指纹操作,请将手指放到指纹模块触摸感应区\r\n");
				/* 参数为0xFFFF进行1:N匹配 */
				id = 0xFFFF;
				fmp_error_code = fpm_idenify_auto(&id);
				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					printf("[app_task_fpm383] password auth success the fingerprint is %04d\r\n", id);

					g_unlock_what = FLAG_UNLOCK_OK;			 // 设备解锁
					g_pass_man_what = FLAG_PASS_MAN_NONE;	 // 解锁成功关闭键盘解锁蓝牙解锁
					g_pass_unlock_mode = MODE_OPEN_LOCK_SFM; // 设置为指纹解锁模式

					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					/* 设置要显示的图片-表情（成功） */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_unlock_icon;

					xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
										 &oled,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend oled picture error code is %d\r\n", xReturn);
					//  发送flash消息记录解锁日志
					flash.mode = g_pass_unlock_mode; // 获取当前解锁模式，存入flash
					xReturn = xQueueSend(g_queue_flash, &flash, 100);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend flash error code is %d\r\n", xReturn);
				}
				else
				{
					fpm_ctrl_led(FPM_LED_RED);
					printf("[app_task_pass_man] password auth fail\r\n");
					/* 设置要显示的图片-表情（失败） */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_error_icon;

					xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
										 &oled,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* 长鸣1秒示意 */
					beep.sta = 1;
					beep.duration = 1000;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);

					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					/* 延时一会 */
					vTaskDelay(2000);
					/* 显示锁屏界面 */
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.pic = pic_lock_icon;
					xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
										 &oled,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
					g_pass_man_what = FLAG_PASS_MAN_NONE; // 回到锁屏界面，等待下一次解锁
				}
				g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // 恢复默认解锁方式

				delay_ms(100);
				fpm_sleep();
				delay_ms(1000);
			}
		}
		if (g_unlock_what == FLAG_UNLOCK_OK) // 设备解锁后才可以继续
		{
			// 添加指纹事件
			if (EventValue & EVENT_GROUP_FPM_ADD)
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				printf("[app_task_fpm383] 执行添加指纹操作,请将手指放到指纹模块触摸感应区\r\n");
				fmp_error_code = fpm_id_total(&id_total);

				if (fmp_error_code == 0)
				{
					printf("[app_task_fpm383] 获取指纹总数：%04d\r\n", id_total);
					/* 添加指纹*/
					fmp_error_code = fpm_enroll_auto(id_total + 1);
					if (fmp_error_code == 0)
					{
						fpm_ctrl_led(FPM_LED_GREEN);
						printf("[app_task_fpm383] 自动注册指纹成功\r\n");
						beep.sta = 1;
						beep.duration = 50;
						xReturn = xQueueSend(g_queue_beep, &beep, 100);
						if (xReturn != pdPASS)
							printf("[app_task_fpm383] xQueueSend  beep error code is %d\r\n", xReturn);
						/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							printf("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					}
					else
					{
						fpm_ctrl_led(FPM_LED_RED);
					}
					delay_ms(100);
					fpm_sleep();
					delay_ms(1000);
				}
			}
			// 删除指纹事件
			if (EventValue & EVENT_GROUP_FPM_DELETE)
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				fmp_error_code = fpm_empty();
				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					printf("[app_task_fpm383] 清空指纹成功\r\n");
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
				}
				else
				{
					fpm_ctrl_led(FPM_LED_RED);
				}
				delay_ms(100);
				fpm_sleep();
				delay_ms(1000);
			}
			// 显示指纹数量事件
			if (EventValue & EVENT_GROUP_FPM_SHOW)
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				fmp_error_code = fpm_id_total(&id_total);

				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					printf("[app_task_fpm383] 获取指纹总数：%04d\r\n", id_total);
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
				}
				else
				{
					fpm_ctrl_led(FPM_LED_RED);
				}
				delay_ms(100);
				fpm_sleep();
				delay_ms(1000);
			}
		}
		vTaskResume(app_task_sr04_handle); // 恢复超声波模块
	}
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

void vApplicationTickHook(void)
{
}
