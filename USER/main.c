#include "includes.h"

#define MAX_DATA_SIZE 512 // 定义足够大的数组大小

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
static TaskHandle_t app_task_fr1002_handle = NULL;
static TaskHandle_t app_task_motor_handle = NULL;
static TaskHandle_t app_task_mqtt_handle = NULL;
static TaskHandle_t app_task_esp8266_handle = NULL;
static TaskHandle_t app_task_monitor_handle = NULL;
static TaskHandle_t app_task_system_reset_handle = NULL;
static TimerHandle_t soft_timer_Handle = NULL; /* 软件定时器句柄 */

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
static void app_task_fr1002(void *pvParameters);	   /*	fr1002人脸识别任务函数*/
static void app_task_motor(void *pvParameters);		   /*	motor步进电机任务函数*/
static void app_task_mqtt(void *pvParameters);		   /* 	mqtt任务函数 */
static void app_task_esp8266(void *pvParameters);	   /*	esp8266无限WIFI模块任务函数*/
static void app_task_monitor(void *pvParameters);	   /*	监控阿里云是否发送消息完成*/
static void app_task_system_reset(void *pvParameters); /*	系统复位任务*/
/* 软件定时器 */
static void soft_timer_callback(TimerHandle_t pxTimer);

/*计数型信号量句柄*/
SemaphoreHandle_t g_sem_led;   // 同步led任务和其他任务
SemaphoreHandle_t g_sem_beep;  // 同步beep任务和其他任务
SemaphoreHandle_t g_sem_motor; // 同步motor任务和其他任务

/*互斥型信号量句柄*/
SemaphoreHandle_t g_mutex_oled;
/* 互斥型信号量句柄 */
SemaphoreHandle_t g_mutex_printf;

/*事件标志组句柄（32bit位标志组）便于管理多个任务标志位*/
EventGroupHandle_t g_event_group;

/*消息队列句柄*/
QueueHandle_t g_queue_usart; // 接收串口发送来的消息
QueueHandle_t g_queue_led;
QueueHandle_t g_queue_beep;
QueueHandle_t g_queue_oled;
QueueHandle_t g_queue_flash;
QueueHandle_t g_queue_keyboard;
QueueHandle_t g_queue_frm;
QueueHandle_t g_queue_motor;
QueueHandle_t g_queue_esp8266; // 用于esp8266WIFI模块的通信

static volatile uint32_t g_esp8266_init = 0;

float g_temp = 0.0;
float g_humi = 0.0;

#define DEBUG_dgb_printf_safe_EN 1

void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_dgb_printf_safe_EN

	va_list args;
	va_start(args, format);

	/* 获取互斥信号量 */
	xSemaphoreTake(g_mutex_printf, portMAX_DELAY);

	vprintf(format, args);

	/* 释放互斥信号量 */
	xSemaphoreGive(g_mutex_printf);

	va_end(args);
#else
	(void)0;
#endif
}

/*全局变量*/
volatile uint32_t g_rtc_get_what = FLAG_RTC_GET_NONE;
volatile uint32_t g_dht_get_what = FLAG_DHT_GET_NONE;
volatile uint32_t g_unlock_what = FLAG_UNLOCK_NO;
volatile uint32_t g_pass_man_what = FLAG_PASS_MAN_NONE;			// 密码管理标志
volatile uint32_t g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // 密码解锁模式。默认键盘解锁，根据不同解锁方式会自动更改该标志位
volatile uint32_t g_system_no_opreation_cnt = 0;
volatile uint32_t g_oled_display_flag = 1;
const char pass_auth_default[PASS_LEN] = {'8', '8', '8', '8', '8', '8'}; // 默认密码
volatile uint32_t g_ISR_dbg = 0;
uint32_t g_key_2_sta = 0;

int main(void)
{
	/* 设置系统中断优先级分组只能选择第4组 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 系统定时器中断频率为configTICK_RATE_HZ 168000000/1000 */
	SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);

	//	/* 初始化串口1 */
	// usart1_init(115200);
	/*初始化串口3*/
	// usart3_init(9600);
	usart6_init(9600);

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
	{app_task_system_reset, "app_task_system_reset", 512, NULL, 5, &app_task_system_reset_handle},
	{app_task_sr04, "app_task_sr04", 1024, NULL, 5, &app_task_sr04_handle},
	{app_task_oled, "app_task_oled", 512, NULL, 5, &app_task_oled_handle},
	{app_task_password_man, "app_task_password_man", 512, NULL, 5, &app_task_password_man_handle},
	{app_task_fpm383, "app_task_fpm383", 512, NULL, 5, &app_task_fpm383_handle},
	{app_task_fr1002, "app_task_fr1002", 1024, NULL, 5, &app_task_fr1002_handle},
	{app_task_motor, "app_task_motor", 512, NULL, 5, &app_task_motor_handle},
	{app_task_mqtt, "app_task_mqtt", 512, NULL, 5, &app_task_mqtt_handle},
	{app_task_monitor, "app_task_monitor", 512, NULL, 5, &app_task_monitor_handle},
	{app_task_esp8266, "app_task_esp8266", 2048, NULL, 5, &app_task_esp8266_handle},
	{0, 0, 0, 0, 0, 0}};

static void app_task_init(void *pvParameters)
{
	uint32_t i = 0;
	//	dgb_printf_safe("app_task_init is running ...\r\n");

	/* 创建互斥型信号量 */
	g_mutex_printf = xSemaphoreCreateMutex();
	/*创建计数型信号量，最大值为10，初值为0*/
	g_sem_led = xSemaphoreCreateCounting(20, 0);
	g_sem_beep = xSemaphoreCreateCounting(20, 0);
	g_sem_motor = xSemaphoreCreateCounting(10, 0);

	/*创建事件标志组*/
	g_event_group = xEventGroupCreate();

	/*创建消息队列,要注意消息队列的容量是否创建正确*/
	g_queue_led = xQueueCreate(QUEUE_LED_LEN, sizeof(uint8_t));					 // 用于led通信的消息队列
	g_queue_beep = xQueueCreate(QUEUE_BEEP_LEN, sizeof(beep_t));				 // 用于beep通信的消息队列
	g_queue_oled = xQueueCreate(QUEUE_OLED_LEN, sizeof(oled_t));				 // 用于oled通信的消息队列
	g_queue_usart = xQueueCreate(QUEUE_USART_LEN, sizeof(usart_t));				 // 用于usart通信的消息队列
	g_queue_flash = xQueueCreate(QUEUE_FLASH_LEN, sizeof(flash_t));				 // 用于flash通信的消息队列
	g_queue_keyboard = xQueueCreate(QUEUE_KEYBOARD_LEN, sizeof(uint8_t));		 // 用于keyboard通信的消息队列
	g_queue_frm = xQueueCreate(QUEUE_FSM_LEN, sizeof(g_usart1_rx_buf));			 // 用于fr1002通信的消息队列
	g_queue_motor = xQueueCreate(QUEUE_MOTOR_LEN, sizeof(uint8_t));				 // 用于motor通信的消息队列
	g_queue_esp8266 = xQueueCreate(QUEUE_ESP8266_LEN, sizeof(g_esp8266_rx_buf)); // 用于esp8266通信的消息队列

	key_init();
	led_init();
	led_breath_init();
	beep_init();
	kbd_init();
	sr04_init();

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
#if IWDG_ENABLE
	/* 独立看门狗初始化 */
	iwdg_init();
#endif

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

	/* 创建周期软件定时器 */
	soft_timer_Handle = xTimerCreate((const char *)"AutoReloadTimer",
									 (TickType_t)1000,	  /* 定时器周期 1000(tick) */
									 (UBaseType_t)pdTRUE, /* 周期模式 */
									 (void *)1,			  /* 为每个计时器分配一个索引的唯一ID */
									 (TimerCallbackFunction_t)soft_timer_callback);
	/* 开启周期软件定时器 */
	xTimerStart(soft_timer_Handle, 0);

	dht11_init();

	OLED_Init();
	fpm_init();
	motor_init();
	OLED_Clear();
	/* 显示logo */
	OLED_DrawBMP(0, 0, 128, 8, (uint8_t *)pic_logo);
	vTaskDelay(3000);
	OLED_Clear();
	OLED_DrawBMP(32, 0, 96, 8, (uint8_t *)pic_lock_icon);

	/* 删除任务自身 */
	vTaskDelete(NULL);
	dgb_printf_safe("[app_task_init] nerver run here\r\n");
}

/*led任务函数*/
static void app_task_led(void *pvParameters)
{
	uint8_t led_sta = 0; // led状态标志位
	BaseType_t xReturn = pdFALSE;
	dgb_printf_safe("[app_task_led] create success\r\n");
	for (;;) // while(1)
	{
		xReturn = xQueueReceive(g_queue_led,	/* 消息队列的句柄 */
								&led_sta,		/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		dgb_printf_safe("[app_task_led] is running ...\r\n");
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
		/* 释放信号量，告诉对方，当前led控制任务已经完成 */
		xSemaphoreGive(g_sem_led);
	}
}
/*呼吸灯模块*/
void app_task_led_breath(void *pvParameters)
{
	int8_t brightness = 0;

	dgb_printf_safe("[app_task_led_breath] create success and suspend self\r\n");
	vTaskSuspend(NULL); // 先挂起呼吸灯
	dgb_printf_safe("[app_task_led_breath] resume success\r\n");

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
	printf("[app_task_beep] create success\r\n");
	for (;;) // while(1)
	{
		xReturn = xQueueReceive(g_queue_beep,	/* 消息队列的句柄 */
								&beep,			/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		dgb_printf_safe("[app_task_beep] is running ...\r\n");
		// 解析消息队列传递来的信息
		if (xReturn != pdPASS)
			continue;
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
	uint32_t time;
	beep_t beep;
	oled_t oled;
	EventBits_t EventValue;
	BaseType_t xReturn = pdFALSE;
	dgb_printf_safe("[app_task_key] create success\r\n");
	for (;;)
	{
		// 等待事件组中的相应事件位，或同步
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_KEYALL_DOWN,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

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
				dgb_printf_safe("[app_task_key] S1 Press\r\n");
				vTaskSuspend(app_task_rtc_handle); // 挂起实时时钟任务

				g_rtc_get_what = FLAG_RTC_GET_NONE; // 不显示时间信息
				if (g_unlock_what == FLAG_UNLOCK_OK)
				{
					// 发送注册指纹事件
					xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_ADD);
				}
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
				dgb_printf_safe("[app_task_key] S2 Press\r\n");

				if (g_unlock_what == FLAG_UNLOCK_NO)
				{
					vTaskSuspend(app_task_rtc_handle);	// 挂起实时时钟任务
					g_rtc_get_what = FLAG_RTC_GET_NONE; // 	不显示时间信息
					// 发送验证指纹事件
					xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_AUTH);
				}
				else if (g_unlock_what == FLAG_UNLOCK_OK)
				{
					time = 0;

					/* 按键长按 */
					while (PEin(2) == 0)
					{
						vTaskDelay(20);

						if (time++ >= 100)
							break;
					}
					// 执行软件复位
					if (time >= 100)
					{
						dgb_printf_safe("[app_task_key] S2 Press Continue system while be reset!!!\r\n");
						/* 发送系统复位事件 */
						xReturn = xEventGroupSetBits(g_event_group, EVENT_GROUP_SYSTEM_RESET);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_key] xEventGroupSetBits EVENT_GROUP_SYSTEM_RESET error code is %d\r\n", xReturn);
					}
					// 按键短按显示时间
					else
					{
						oled.ctrl = OLED_CTRL_CLEAR;
						xReturn = xQueueSend(g_queue_oled, &oled, 100);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_key] xQueuesend oled error code is %d\r\n", xReturn);

						// 恢复RTC时钟
						vTaskResume(app_task_rtc_handle);
						g_key_2_sta = !g_key_2_sta;
						dgb_printf_safe("g_key_2_sta:%d", g_key_2_sta);
						g_rtc_get_what = (g_key_2_sta > 0) ? FLAG_RTC_GET_DATE : FLAG_RTC_GET_TIME;
						dgb_printf_safe("g_rtc_get_what:%d", g_rtc_get_what);
					}
				}
			}
			// 等待按键释放
			while (PEin(2) == 0)
			{
				vTaskDelay(1);
			}

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
				dgb_printf_safe("[app_task_key] S1 Press\r\n");

				g_rtc_get_what = FLAG_RTC_GET_NONE;
				if (g_unlock_what == FLAG_UNLOCK_OK)
				{
					// 发送显示指纹数量事件
					xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_SHOW);
				}
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
				dgb_printf_safe("[app_task_key] S4 Press\r\n");
				if (g_unlock_what == FLAG_UNLOCK_OK)
				{
					// 发送删除指纹事件
					xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_DELETE);
				}
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
	dgb_printf_safe("[app_task_kbd] create success\r\n");
	while (1)
	{
		/* 读取矩阵键盘按键值 */
		key_val = kbd_read();

		if (key_val != 'N')
		{
			dgb_printf_safe("[app_task_kbd] kbd press %c \r\n", key_val);

			xReturn = xQueueSend(g_queue_keyboard, &key_val, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_kbd] xQueueSend kbd error code is %d\r\n", xReturn);

			/* 嘀一声示意 */
			beep.sta = 1;
			beep.duration = 10;
			xReturn = xQueueSend(g_queue_beep, &beep, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_kbd] xQueueSend beep error code is %d\r\n", xReturn);

			/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_key] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
		}
	}
}
/*rtc实时时钟模块*/
static void app_task_rtc(void *pvParameters)
{
	uint8_t buf[16] = {0};
	uint32_t i;
	beep_t beep;
	oled_t oled;
	BaseType_t xReturn;
	EventBits_t EventValue;

	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	dgb_printf_safe("[app_task_rtc] create success and suspend self\r\n");
	vTaskSuspend(app_task_rtc_handle);
	dgb_printf_safe("[app_task_rtc] resume success\r\n");

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_RTC_WAKEUP | EVENT_GROUP_RTC_ALARM,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		if ((EventValue & EVENT_GROUP_RTC_WAKEUP) && g_oled_display_flag&&(g_unlock_what==FLAG_UNLOCK_OK)) // RTC事件被触发,亮屏状态才在oled上显示
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
			if (g_oled_display_flag)
			{
				/* oled显示时间 */
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.y = 4;
				oled.str = buf; // 字符串缓冲区首地址
				oled.font_size = 16;

				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			}
		}
		if (EventValue & EVENT_GROUP_RTC_ALARM) // 闹钟被唤醒事件
		{
			dgb_printf_safe("[app_task_key] 闹钟唤醒了\r\n");
			beep.duration = 50;
			beep.sta = 1;
			for (i = 0; i < 10; i++)
			{
				xReturn = xQueueSend(g_queue_beep, &beep, 100);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend beep error code is %d\r\n", xReturn);
				/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
				xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
				vTaskDelay(300);
				xReturn = xQueueSend(g_queue_beep, &beep, 100);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend beep error code is %d\r\n", xReturn);
				/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
				xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
				vTaskDelay(500);
			}
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

	BaseType_t xReturn;
	EventBits_t EventValue;
	dgb_printf_safe("[app_task_dht] create success\r\n");
	for (;;) // while(1)
	{
		// 等待事件组中的相应事件位，或同步
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_DHT,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		if (EventValue & EVENT_GROUP_DHT)
		{
			dgb_printf_safe("[app_task_dht] is running ...\r\n");
			if ((g_unlock_what == FLAG_UNLOCK_OK) && (g_dht_get_what == FLAG_DHT_GET_START))
			{
				// 挂起别的模块
				vTaskSuspend(app_task_sr04_handle); // 挂起超声波模块

				delay_ms(5);

				dht11_read_ret = dht11_read(dht11_buf);

				if (dht11_read_ret == 0)
				{
					memset(&oled, 0, sizeof(oled));
					g_temp = (float)dht11_buf[2] + (float)dht11_buf[3] / 10;
					g_humi = (float)dht11_buf[0] + (float)dht11_buf[1] / 10;

					dgb_printf_safe("Temperature=%.1f Humidity=%.1f\r\n", g_temp, g_humi);

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
						dgb_printf_safe("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
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
						dgb_printf_safe("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
				}
				else
				{
					dgb_printf_safe("[app_task_dht] dht11 read error code is %d\r\n", dht11_read_ret);
				}
			}
			vTaskResume(app_task_sr04_handle); // 恢复超声波模块
		}
	}
}
/*超声波模块*/
static void app_task_sr04(void *pvParameters)
{
	int32_t distance = 0;
	oled_t oled;
	EventBits_t xReturn;
	EventBits_t EventValue = 0;
	dgb_printf_safe("[app_task_sr04] create success\r\n");
	for (;;) // while(1)
	{

		dgb_printf_safe("[app_task_sr04] is running ...\r\n");
		distance = sr04_get_distance();
		dgb_printf_safe("[app_task_sr04] distance = %d\r\n", distance);
		if (distance > 400)
		{
			oled.ctrl = OLED_CTRL_DISPLAY_OFF; /*息屏*/
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_sr04] xQueueSend oled string error code is %d\r\n", xReturn);
		}

		if (distance >= 20 && distance <= 400)
		{
			// 亮屏
			oled.ctrl = OLED_CTRL_DISPLAY_ON;
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_sr04] xQueueSend oled string error code is %d\r\n", xReturn);
			if (g_unlock_what == FLAG_UNLOCK_NO) // 设备上锁期间才能进行人脸识别
			{									 // 发送验证人脸事件
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FACE_AUTH);
				// 等待人脸识别成功标志
				/* 等待人脸识别完毕 */
				dgb_printf_safe("[app_task_sr04] xEventGroupSetBits wait...\r\n");
				EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
												 (EventBits_t)EVENT_GROUP_FACE_OK | EVENT_GROUP_FACE_AGAIN,
												 (BaseType_t)pdTRUE,
												 (BaseType_t)pdFALSE,
												 (TickType_t)portMAX_DELAY);

				if (EventValue & EVENT_GROUP_FACE_OK) // 人脸验证成功
				{
					while (1)
					{
						distance = sr04_get_distance();
						if (distance >= 20 && distance <= 400)
						{
							dgb_printf_safe("[app_task_sr04] 人脸识别成功了,请开门...\r\n");
						}
						else if (distance > 400)
							break;
						delay_ms(500);
					}
				}
				/*人脸识别失败，重新触发*/
				if (EventValue & EVENT_GROUP_FACE_AGAIN) // 人脸验证失败或者设备重新上锁
				{
					dgb_printf_safe("[app_task_sr04] 人脸识别失败再次触发人脸识别...\r\n");
					delay_ms(500);
				}
			}
		}

		delay_ms(500);
	}
}
/*oled模块*/
static void app_task_oled(void *pvParameters)
{
	oled_t oled;
	BaseType_t xReturn = pdFALSE;
	dgb_printf_safe("[app_task_oled] create success\r\n");
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
			dgb_printf_safe("[app_task_oled] oled ctrl code is invalid\r\n");
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
	dgb_printf_safe("[app_task_flash] create success\r\n");
	for (;;)
	{
		xReturn = xQueueReceive(g_queue_flash,	  /* 消息队列的句柄 */
								&flash_write_buf, /* 接收的消息内容 */
								portMAX_DELAY);	  /* 等待时间一直等 */
		if (xReturn != pdPASS)
			continue;

		dgb_printf_safe("[app_task_flash] wait get data\r\n");

		// 获取当前扇区写入块标识
		flash_write_buf.offset = flash_read_offset((uint32_t *)(0x08010000));
		if (flash_write_buf.offset >= 20)
		{
			flash_init();				// 清空数据
			flash_write_buf.offset = 0; // 从0开始重新写入
		}

		RTC_GetDate(RTC_Format_BCD, &RTC_Date);
		RTC_GetTime(RTC_Format_BCD, &RTC_Time);

		dgb_printf_safe("[app_task_flash] 当前时间:20%02x/%02x/%02x %02x:%02x:%02x %d\n",
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
	oled_t oled;

	BaseType_t xReturn = pdFALSE;

	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;

	dgb_printf_safe("[app_task_usart] create success\r\n");
	for (;;)
	{
		dgb_printf_safe("[app_task_usart] wait info ...\r\n");
		// 消息队列为空的时候会阻塞，不需要用事件标志组触发任务
		xReturn = xQueueReceive(g_queue_usart,	/* 消息队列的句柄 */
								&usart_packet,	/* 得到的消息内容 */
								portMAX_DELAY); /* 等待时间一直等 */
		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[app_task_usart] xQueueReceive usart_packet error code is %d\r\n", xReturn);
			continue;
		}

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		// 解析消息
		dgb_printf_safe("[app_task_usart] recv data:%s\r\n", usart_packet.rx_buf);

		// 获取当前温湿度
		if (strstr((char *)usart_packet.rx_buf, "TEMP1"))
		{
			vTaskSuspend(app_task_rtc_handle);	 // 挂起实时时钟任务
			g_rtc_get_what = FLAG_RTC_GET_NONE;	 // 	不显示时间信息
			if (g_unlock_what == FLAG_UNLOCK_OK) // 设备已经解锁
			{
				oled.ctrl = OLED_CTRL_CLEAR;
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
				g_dht_get_what = FLAG_DHT_GET_START; // 开始获取温度
			}
		}
		// 停止获取当前温湿度
		if (strstr((char *)usart_packet.rx_buf, "TEMP0"))
		{
			vTaskSuspend(app_task_rtc_handle);	 // 挂起实时时钟任务
			g_rtc_get_what = FLAG_RTC_GET_NONE;	 // 	不显示时间信息
			if (g_unlock_what == FLAG_UNLOCK_OK) // 设备已经解锁
			{
				oled.ctrl = OLED_CTRL_CLEAR;
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
				g_dht_get_what = FLAG_DHT_GET_NONE; // 停止获取温度
			}
		}
		if (strstr((char *)usart_packet.rx_buf, "ALARM SET")) // 设置闹钟：日-时-分-秒
		{
			// 截取 日-时-分-秒
			p = strtok((char *)usart_packet.rx_buf, "-");
			// dgb_printf_safe("p:%s\r\n",p);
			date = to_hex(atoi(strtok(NULL, "-")));
			// dgb_printf_safe("date:%d\r\n",date);
			hours = to_hex(atoi(strtok(NULL, "-")));
			// dgb_printf_safe("hours:%d\r\n",hours);
			minutes = to_hex(atoi(strtok(NULL, "-")));
			// dgb_printf_safe("minutes:%d\r\n",minutes);
			seconds = to_hex(atoi(strtok(NULL, "#")));
			// dgb_printf_safe("seconds:%d\r\n",seconds);
			rtc_alarm_init(hours, minutes, seconds, date); // 设置
			dgb_printf_safe("[app_task_usart] rtc set alarm ok\r\n");
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
			dgb_printf_safe("[app_task_usart] rtc set date ok\r\n");
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

			dgb_printf_safe("[app_task_usart] rtc set time ok\r\n");
		}
		else if (strstr((char *)usart_packet.rx_buf, "PASS UNLOCK") && (g_pass_man_what == FLAG_PASS_MAN_AUTH)) // 验证密码
		{
			p = strtok((char *)usart_packet.rx_buf, "PASS UNLOCK");
			dgb_printf_safe("[app_task_usart] recv pass :%s\r\n", p);
			g_pass_unlock_mode = MODE_OPEN_LOCK_BLUE; // 蓝牙解锁模式
			for (i = 0; i < 8; i++)
			{
				key_val = p[i];
				xReturn = xQueueSend(g_queue_keyboard, /* 消息队列的句柄 */
									 &key_val,		   /* 发送的消息内容 */
									 100);			   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_usart] xQueueSend kbd error code is %d\r\n", xReturn);
			}
		}
		if (strstr((char *)usart_packet.rx_buf, "LOG"))
		{
			flash_read(20);
		}
		if (strstr((char *)usart_packet.rx_buf, "TIME?")) // 显示实时时间
		{
			oled.ctrl = OLED_CTRL_CLEAR; /* oled清屏 */
			xReturn = xQueueSend(g_queue_oled, &oled, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_key] xQueueSend oled string error code is %d\r\n", xReturn);
			vTaskResume(app_task_rtc_handle);	// 恢复rtc任务
			g_rtc_get_what = FLAG_RTC_GET_TIME; // 获取时间
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
	uint8_t motor;
	oled_t oled;
	beep_t beep;
	flash_t flash;
	BaseType_t xReturn;
	dgb_printf_safe("[app_task_pass_man] create success\r\n");
	for (;;)
	{
		// 等待矩阵键盘消息队列消息
		xReturn = xQueueReceive(g_queue_keyboard, &key_val, portMAX_DELAY);
		if (xReturn != pdPASS)
			continue;

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		if (key_val == 'A')
		{
			// 发送注册人脸事件
			xEventGroupSetBits(g_event_group, EVENT_GROUP_FACE_ADD);
		}
		if (key_val == 'B')
		{
			// 发送查询人脸个数事件
			xEventGroupSetBits(g_event_group, EVENT_GROUP_FACE_SHOW);
		}
		if (key_val == 'D')
		{
			// 发送删除人脸事件
			xEventGroupSetBits(g_event_group, EVENT_GROUP_FACE_DELETE);
		}

		if (key_val == 'C') // 关锁
		{
			dgb_printf_safe("[app_task_pass_man] password reset\r\n");

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
				dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
			oled.ctrl = OLED_CTRL_SHOW_STRING;
			oled.x = x;
			oled.y = y;
			oled.font_size = 16;
			oled.str = "------";
			xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
								 &oled,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

			if (g_unlock_what == FLAG_UNLOCK_OK) // 设备解锁状态才能关锁
			{
				g_unlock_what = FLAG_UNLOCK_NO; // 设备上锁

				motor = MOTOR_DOUBLE_REV;
				xReturn = xQueueSend(g_queue_motor, &motor, 100);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_pass_man] xQueueSend motor error code is %d\r\n", xReturn);
				/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
				xReturn = xSemaphoreTake(g_sem_motor, portMAX_DELAY);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_motor error code is %d\r\n", xReturn);

				xEventGroupSetBits(g_event_group, EVENT_GROUP_FACE_AGAIN); // 发送设备上锁从新验证人脸事件
			}

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
					dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
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
					dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

				vTaskDelay(100);
				/* 显示* */
				oled.str = "*";
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

				x += 8;
				key_cnt++;
			}
			else if (key_val == '#' && key_cnt >= 6)
			{
				dgb_printf_safe("[app_task_pass_man] auth key buf is %6s\r\n", key_buf);

				/* 读取eeprom存储的密码 */
				// at24c02_read(PASS1_ADDR_IN_EEPROM, (uint8_t *)pass_auth_eeprom, PASS_LEN);

				/* 匹配默认密码 */
				rt = memcmp(pass_auth_default, key_buf, PASS_LEN);
				/* 密码匹配成功 */
				if (rt == 0)
				{
					dgb_printf_safe("[app_task_pass_man] password auth success\r\n");
					g_unlock_what = FLAG_UNLOCK_OK;		  // 设备解锁
					g_pass_man_what = FLAG_PASS_MAN_NONE; // 不再进行密码验证

					motor = MOTOR_DOUBLE_POS;
					xReturn = xQueueSend(g_queue_motor, &motor, 100);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_pass_man] xQueueSend motor error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_motor, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_motor error code is %d\r\n", xReturn);

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
						dgb_printf_safe("[app_task_pass_man] xQueueSend flash error code is %d\r\n", xReturn);
				}
				else
				{
					dgb_printf_safe("[app_task_pass_man] password auth fail\r\n");
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
						dgb_printf_safe("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* 长鸣1秒示意 */
					beep.sta = 1;
					beep.duration = 1000;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);

					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);

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
						dgb_printf_safe("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
				}
				g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // 恢复默认解锁方式
			}
		}

		if (g_pass_man_what == FLAG_PASS_MAN_MODIFY) // 密码修改模式
		{
		}
	}
}
/*指纹识别模块*/
void app_task_fpm383(void *pvParameters)
{
	uint8_t fmp_error_code;
	uint16_t id;
	uint16_t id_total;
	uint8_t motor;
	beep_t beep;
	oled_t oled;
	flash_t flash;
	EventBits_t EventValue;
	BaseType_t xReturn = pdFALSE;
	dgb_printf_safe("[app_task_fpm383] create success\r\n");
	for (;;)
	{
		// 等待事件组中的相应事件位，或同步
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_FPM_ADD | EVENT_GROUP_FPM_AUTH | EVENT_GROUP_FPM_DELETE | EVENT_GROUP_FPM_SHOW,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		vTaskSuspend(app_task_sr04_handle); // 挂起超声波模块
		vTaskSuspend(app_task_dht_handle);	// 挂起温湿度模块

		if (g_unlock_what == FLAG_UNLOCK_NO) // 设备上锁后才能解锁
		{
			if (EventValue & EVENT_GROUP_FPM_AUTH) // 验证指纹事件
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				dgb_printf_safe("[app_task_fpm383] 执行刷指纹操作,请将手指放到指纹模块触摸感应区\r\n");
				/* 参数为0xFFFF进行1:N匹配 */
				id = 0xFFFF;
				fmp_error_code = fpm_idenify_auto(&id);
				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					dgb_printf_safe("[app_task_fpm383] password auth success the fingerprint is %04d\r\n", id);

					g_unlock_what = FLAG_UNLOCK_OK;			 // 设备解锁
					g_pass_man_what = FLAG_PASS_MAN_NONE;	 // 解锁成功关闭键盘解锁蓝牙解锁
					g_pass_unlock_mode = MODE_OPEN_LOCK_SFM; // 设置为指纹解锁模式

					motor = MOTOR_DOUBLE_POS;
					xReturn = xQueueSend(g_queue_motor, &motor, 100);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xQueueSend motor error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对motor的控制 */
					xReturn = xSemaphoreTake(g_sem_motor, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xSemaphoreTake g_sem_motor error code is %d\r\n", xReturn);

					//  发送flash消息记录解锁日志
					flash.mode = g_pass_unlock_mode; // 获取当前解锁模式，存入flash
					xReturn = xQueueSend(g_queue_flash, &flash, 100);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xQueueSend flash error code is %d\r\n", xReturn);
				}
				else
				{
					fpm_ctrl_led(FPM_LED_RED);
					dgb_printf_safe("[app_task_fpm383] password auth fail\r\n");
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
						dgb_printf_safe("[app_task_fpm383] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* 长鸣1秒示意 */
					beep.sta = 1;
					beep.duration = 1000;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);

					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
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
						dgb_printf_safe("[app_task_fpm383] xQueueSend oled string error code is %d\r\n", xReturn);
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
				dgb_printf_safe("[app_task_fpm383] 执行添加指纹操作,请将手指放到指纹模块触摸感应区\r\n");
				fmp_error_code = fpm_id_total(&id_total);

				if (fmp_error_code == 0)
				{
					dgb_printf_safe("[app_task_fpm383] 获取指纹总数：%04d\r\n", id_total);
					/* 添加指纹*/
					fmp_error_code = fpm_enroll_auto(id_total + 1);
					if (fmp_error_code == 0)
					{
						fpm_ctrl_led(FPM_LED_GREEN);
						dgb_printf_safe("[app_task_fpm383] 自动注册指纹成功\r\n");
						beep.sta = 1;
						beep.duration = 50;
						xReturn = xQueueSend(g_queue_beep, &beep, 100);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fpm383] xQueueSend  beep error code is %d\r\n", xReturn);
						/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
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
					dgb_printf_safe("[app_task_fpm383] 清空指纹成功\r\n");
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
				}
				else
				{
					fpm_ctrl_led(FPM_LED_RED);
				}
				delay_ms(100);
				fpm_sleep();
				delay_ms(1000);
				vTaskResume(app_task_sr04_handle); // 恢复超声波模块
				vTaskResume(app_task_dht_handle);  // 恢复温湿度模块
			}
			// 显示指纹数量事件
			if (EventValue & EVENT_GROUP_FPM_SHOW)
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				fmp_error_code = fpm_id_total(&id_total);

				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					dgb_printf_safe("[app_task_fpm383] 获取指纹总数：%04d\r\n", id_total);
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
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
		vTaskResume(app_task_dht_handle);  // 恢复温湿度模块
	}
}
/*人脸识别模块*/
void app_task_fr1002(void *pvParameters)
{
	uint8_t buf[64] = {0};
	int32_t user_total;
	int32_t distance = 0;
	uint32_t t = 0;
	beep_t beep;
	oled_t oled;
	flash_t flash;
	EventBits_t EventValue;
	BaseType_t xReturn = pdFALSE;
	dgb_printf_safe("[app_task_fr1002] create success\r\n");
	while (fr_init(115200) != 0)
	{
		delay_ms(1000);
		dgb_printf_safe("[app_task_fr1002] 3D人脸识别模块连接中 ...\r\n");
	}
	dgb_printf_safe("[app_task_fr1002] 3D人脸识别模块已连接上\r\n");

	for (;;)
	{
		// 等待事件组中的相应事件位，或同步
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_FACE_ADD | EVENT_GROUP_FACE_AUTH | EVENT_GROUP_FACE_DELETE | EVENT_GROUP_FACE_SHOW,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		dgb_printf_safe("[app_task_fr1002] is running...\r\n");

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		if (g_unlock_what == FLAG_UNLOCK_NO) // 设备上锁后才能解锁
		{
			if (EventValue & EVENT_GROUP_FACE_AUTH)
			{
				dgb_printf_safe("[ap_task_fr1002] 执行验证人脸操作\r\n");
				memset(buf, 0, sizeof buf);
				if (0 == fr_match(buf))
				{
					dgb_printf_safe("[app_task_fr1002] 人脸匹配成功!\r\n");
					// 发送验证人脸成功事件
					if (g_unlock_what == FLAG_UNLOCK_NO)
					{
						xEventGroupSetBits(g_event_group, EVENT_GROUP_FACE_OK);
						g_unlock_what = FLAG_UNLOCK_OK;
						g_pass_unlock_mode = MODE_OPEN_LOCK_FRM;
						/*显示解锁成功界面*/
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
							dgb_printf_safe("[app_task_fr1002] xQueueSend oled picture error code is %d\r\n", xReturn);

						/* 嘀一声示意:第一次 */
						beep.sta = 1;
						beep.duration = 50;
						xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
											 &beep,		   /* 发送的消息内容 */
											 100);		   /* 等待时间 100 Tick */
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fr1002] xQueueSend beep error code is %d\r\n", xReturn);
						/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fr1002] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
						vTaskDelay(100);
						/* 嘀一声示意:第二次 */
						beep.sta = 1;
						beep.duration = 50;
						xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
											 &beep,		   /* 发送的消息内容 */
											 100);		   /* 等待时间 100 Tick */
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fr1002] xQueueSend beep error code is %d\r\n", xReturn);
						/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fr1002] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
						/* 延时一会 */
						vTaskDelay(2000);
						//  发送flash消息记录解锁日志
						flash.mode = g_pass_unlock_mode; // 获取当前解锁模式，有蓝牙解锁、键盘解锁，存入flash
						xReturn = xQueueSend(g_queue_flash, &flash, 100);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fr1002] xQueueSend flash error code is %d\r\n", xReturn);
					}
				}
				else
				{
					dgb_printf_safe("[app_task_fr1002] 人脸匹配失败!\r\n");
					// 发送验证人脸失败并需要再次验证事件
					if (g_unlock_what == FLAG_UNLOCK_NO)
					{
						xEventGroupSetBits(g_event_group, EVENT_GROUP_FACE_AGAIN);
						/*显示人脸解锁失败界面*/
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
							dgb_printf_safe("[app_task_fr1002] xQueueSend oled picture error code is %d\r\n", xReturn);

						/* 长鸣1秒示意 */
						beep.sta = 1;
						beep.duration = 1000;
						xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
											 &beep,		   /* 发送的消息内容 */
											 100);		   /* 等待时间 100 Tick */
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fr1002] xQueueSend beep error code is %d\r\n", xReturn);

						/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
						xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
						if (xReturn != pdPASS)
							dgb_printf_safe("[app_task_fr1002] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
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
							dgb_printf_safe("[app_task_fr1002] xQueueSend oled string error code is %d\r\n", xReturn);
					}
				}
				delay_ms(500);
				/* 进入掉电模式 */
				fr_power_down();
				delay_ms(500);
				g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // 恢复默认解锁方式
			}
		}

		if (g_unlock_what == FLAG_UNLOCK_OK) // 执行其他操作
		{
			if (EventValue & EVENT_GROUP_FACE_ADD) // 添加人脸
			{
				dgb_printf_safe("[ap_task_fr1002] 执行添加人脸操作\r\n");
				if (0 == fr_reg_user("twen"))
				{
					dgb_printf_safe("[app_task_fr1002] 注册用户成功!\r\n");
					/* 嘀一声示意:第一次 */
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fr1002] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fr1002] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					/* 嘀一声示意:第二次 */
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fr1002] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fr1002] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
				}
				else
				{
					dgb_printf_safe("[app_task_fr1002] 注册用户失败!\r\n");
					/* 嘀一声示意	*/
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
										 &beep,		   /* 发送的消息内容 */
										 100);		   /* 等待时间 100 Tick */
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fr1002] xQueueSend beep error code is %d\r\n", xReturn);
					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_fr1002] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
				}
				delay_ms(500);
				/* 进入掉电模式 */
				fr_power_down();
				delay_ms(500);
			}

			if (EventValue & EVENT_GROUP_FACE_SHOW) // 查询人脸个数
			{
				user_total = fr_get_user_total();

				if (user_total < 0)
				{
					dgb_printf_safe("[app_task_fr1002] 获取已注册的用户总数失败!\r\n");
				}
				else
				{
					dgb_printf_safe("[app_task_fr1002] 获取已注册的用户总数:%d\r\n", user_total);
				}
				delay_ms(500);
				/* 进入掉电模式 */
				fr_power_down();
				delay_ms(500);
			}

			if (EventValue & EVENT_GROUP_FACE_DELETE) // 删除人脸
			{
				if (0 == fr_del_user_all())
				{
					dgb_printf_safe("[app_task_fr1002] 删除所有用户成功!\r\n");
					// beep_on();delay_ms(100);beep_off();
				}
				else
				{
					dgb_printf_safe("[app_task_fr1002] 删除所有用户失败!\r\n");
				}
				delay_ms(500);
				/* 进入掉电模式 */
				fr_power_down();
				delay_ms(500);
			}
		}
	}
}

/*步进电机模块*/
void app_task_motor(void *pvParameters)
{
	beep_t beep;
	uint8_t motor;
	oled_t oled;
	BaseType_t xReturn;
	for (;;)
	{
		xReturn = xQueueReceive(g_queue_motor, &motor, portMAX_DELAY);
		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[app_task_motor] xQueueReceive usart_packet error code is %d\r\n", xReturn);
			continue;
		}
		if (motor == MOTOR_DOUBLE_POS) // 解锁
		{
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
				dgb_printf_safe("[app_task_motor] xQueueSend oled picture error code is %d\r\n", xReturn);
			motor_corotation_double_pos();
			/* 嘀一声示意:第一次 */
			beep.sta = 1;
			beep.duration = 20;
			xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
								 &beep,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_motor] xQueueSend beep error code is %d\r\n", xReturn);
			/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_motor] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
			vTaskDelay(100);
			/* 嘀一声示意:第二次 */
			beep.sta = 1;
			beep.duration = 20;
			xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
								 &beep,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_motor] xQueueSend beep error code is %d\r\n", xReturn);
			/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_motor] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
		}

		if (motor == MOTOR_DOUBLE_REV)
		{
			g_rtc_get_what = FLAG_RTC_GET_NONE; // 	不显示时间信息
			vTaskSuspend(app_task_rtc_handle);	// 挂起实时时钟任务
			g_dht_get_what = FLAG_DHT_GET_NONE; // 只要设备上锁就停止获取温湿度

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
				dgb_printf_safe("[app_task_motor] xQueueSend oled string error code is %d\r\n", xReturn);
			motor_corotation_double_rev();
			/* 长鸣1秒示意 */
			beep.sta = 1;
			beep.duration = 1000;
			xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
								 &beep,		   /* 发送的消息内容 */
								 100);		   /* 等待时间 100 Tick */
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_motor] xQueueSend beep error code is %d\r\n", xReturn);

			/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_motor] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
		}
		xSemaphoreGive(g_sem_motor);
		delay_ms(5000);
	}
}

/*MQTT模块*/
void app_task_mqtt(void *pvParameters)
{
	uint32_t delay_1s_cnt = 0;
	uint8_t buf[5] = {20, 05, 56, 8, 20};

	dgb_printf_safe("[app_task_mqtt] create success\r\n");

	dgb_printf_safe("[app_task_mqtt] suspend\r\n");

	vTaskSuspend(NULL); // 挂起自己，等待esp8266初始化完成将自己恢复

	dgb_printf_safe("[app_task_mqtt] resume\r\n");

	vTaskDelay(1000);

	for (;;)
	{
		// 发送心跳包
		mqtt_send_heart();

		// 上报设备状态
		mqtt_report_devices_status();

		delay_ms(1000);

		delay_1s_cnt++;

		if (delay_1s_cnt >= 6) // 每6s触发一次温湿度记录事件
		{
			delay_1s_cnt = 0;
			if (g_dht_get_what == FLAG_DHT_GET_START)
				xEventGroupSetBits(g_event_group, EVENT_GROUP_DHT); // 触发事件标志位0x20,触发温度记录任务
		}
	}
}
/*管理mqtt和esp8266函数，当esp8266在一定时间内没有接收数据（可以换成定时器？满足条件触发一个事件），就将接收到的数据发送到esp8266消息队列，由esp8266任务函数解析处理，*/
void app_task_monitor(void *pvParameters)
{
	uint32_t esp8266_rx_cnt = 0;

	BaseType_t xReturn = pdFALSE;

	dgb_printf_safe("[app_task_monitor] create success \r\n");

	for (;;)
	{
		esp8266_rx_cnt = g_esp8266_rx_cnt;

		delay_ms(10);

		/* n毫秒后，发现g_esp8266_rx_cnt没有变化，则认为接收数据结束 */
		if (g_esp8266_init && esp8266_rx_cnt && (esp8266_rx_cnt == g_esp8266_rx_cnt))
		{
			/* 发送消息，如果队列满了，超时时间为1000个节拍，如果1000个节拍都发送失败，函数直接返回 */
			xReturn = xQueueSend(g_queue_esp8266, (void *)g_esp8266_rx_buf, 1000);

			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_monitor] xQueueSend g_queue_esp8266 error code is %d\r\n", xReturn);

			g_esp8266_rx_cnt = 0;
			memset((void *)g_esp8266_rx_buf, 0, sizeof(g_esp8266_rx_buf));
		}
	}
}

void app_task_system_reset(void *pvParameters)
{

	uint8_t x = 0;
	beep_t beep;
	oled_t oled;
	BaseType_t xReturn = pdFALSE;
	EventBits_t EventValue;
	uint32_t i = 0;

	dgb_printf_safe("[app_task_system_reset] create success\r\n");

	for (;;)
	{
		/* 等待事件组中的相应事件位，或同步 */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_SYSTEM_RESET,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		if (EventValue & EVENT_GROUP_SYSTEM_RESET)
		{
			/* 挂起涉及用到OLED的任务 */
			// vTaskSuspend(app_task_key_handle);

			vTaskSuspend(app_task_dht_handle);
			vTaskSuspend(app_task_rtc_handle);

			dgb_printf_safe("[app_task_system_reset] xQueueSend oled clear\r\n");

			/* OLED清屏 */
			oled.ctrl = OLED_CTRL_CLEAR;
			xQueueSend(g_queue_oled, &oled, 100);

			/* 显示"正在执行复位" */
			oled.ctrl = OLED_CTRL_SHOW_CHINESE;
			oled.y = 2;

			for (x = 16, i = 16; i <= 21; i++, x += 16)
			{
				oled.x = x;
				oled.chinese = i;
				xQueueSend(g_queue_oled, &oled, 100);
			}

			x = 0;
			oled.ctrl = OLED_CTRL_SHOW_STRING;
			oled.y = 6;
			oled.font_size = 16;

			while (1)
			{
				/* 显示进度 */
				oled.x = x;
				oled.str = ">";
				xQueueSend(g_queue_oled, &oled, 100);

				vTaskDelay(100);

				x += 8;

				if (x >= 120)
				{
					/* 嘀一声示意 */
					beep.sta = 1;
					beep.duration = 100;

					xQueueSend(g_queue_beep, /* 消息队列的句柄 */
							   &beep,		 /* 发送的消息内容 */
							   100);		 /* 等待时间 100 Tick */

					/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						dgb_printf_safe("[app_task_system_reset] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);

					/* OLED清屏 */
					oled.ctrl = OLED_CTRL_CLEAR;
					xQueueSend(g_queue_oled, &oled, 100);

					/* 系统复位 */
					NVIC_SystemReset();
				}
			}
		}
	}
}
/*软件定时器模块*/
void soft_timer_callback(TimerHandle_t pxTimer)
{
	static uint32_t count_backwards_sta = 0;
	static uint32_t oled_display_what = FLAG_RTC_GET_NONE;
	oled_t oled;
	BaseType_t xReturn;
	uint8_t buf[16] = {0};
	int32_t timer_id = (int32_t)pvTimerGetTimerID(pxTimer);
	// dgb_printf_safe("g_system_no_opreation_cnt : %d\r\n", g_system_no_opreation_cnt);

	/* 倒数状态成立 且 统计系统无操作计数值为0 或 g_system_no_opreation_cnt达到熄屏时间值 */
	if (count_backwards_sta && (g_system_no_opreation_cnt >= SCREEN_OFF_TIME))
	{
		/* 清空记录倒数状态 */
		count_backwards_sta = 0;

		/* oled显示字符串 */
		/* 清空右上角的倒数值*/
		oled.ctrl = OLED_CTRL_SHOW_STRING;
		oled.x = 120;
		oled.y = 0;

		memset(buf, 0, sizeof buf);
		sprintf((char *)buf, "%d", SCREEN_OFF_TIME - g_system_no_opreation_cnt);

		oled.str = " ";
		oled.font_size = 12;

		xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							 &oled,		   /* 发送的消息内容 */
							 100);		   /* 等待时间 100 Tick */
		if (xReturn != pdPASS)
			dgb_printf_safe("[soft_timer_callback id=%d] xQueueSend oled string error code is %d\r\n", timer_id, xReturn);
	}

	/* 开始倒数,3 2 1 */
	if ((g_system_no_opreation_cnt >= SCREEN_OFF_TIME - 3) && (g_system_no_opreation_cnt < SCREEN_OFF_TIME))
	{
		dgb_printf_safe("[soft_timer_callback id=%d] g_system_no_opreation_cnt is %d\r\n", timer_id, g_system_no_opreation_cnt);

		/* 记录倒数状态 */
		count_backwards_sta = 1;

		/* oled显示字符串 */
		oled.ctrl = OLED_CTRL_SHOW_STRING;
		oled.x = 120;
		oled.y = 0;

		memset(buf, 0, sizeof buf);
		sprintf((char *)buf, "%d", SCREEN_OFF_TIME - g_system_no_opreation_cnt);

		oled.str = buf;
		oled.font_size = 12;

		xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							 &oled,		   /* 发送的消息内容 */
							 100);		   /* 等待时间 100 Tick */
		if (xReturn != pdPASS)
			dgb_printf_safe("[soft_timer_callback id=%d] xQueueSend oled string error code is %d\r\n", timer_id, xReturn);
	}

	/* g_system_no_opreation_cnt达到熄屏时间值，则执行OLED熄屏*/
	if (g_oled_display_flag && (g_system_no_opreation_cnt >= SCREEN_OFF_TIME))
	{
		g_oled_display_flag = 0;
		oled_display_what = g_rtc_get_what; // 记录熄屏之前oled上显示的内容

		g_rtc_get_what = FLAG_RTC_GET_NONE; // 关闭rtc显示
		/* OLED熄屏控制 */
		oled.ctrl = OLED_CTRL_DISPLAY_OFF;

		xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							 &oled,		   /* 发送的消息内容 */
							 100);		   /* 等待时间 100 Tick */
		if (xReturn != pdPASS)
			dgb_printf_safe("[soft_timer_callback id=%d] xQueueSend oled error code is %d\r\n", timer_id, xReturn);
	}
	else if ((g_system_no_opreation_cnt == 0) && (g_oled_display_flag == 0))
	{
		g_oled_display_flag = 1;

		g_rtc_get_what = oled_display_what; // 复原熄屏前的显示内容
		/* OLED亮屏控制 */
		oled.ctrl = OLED_CTRL_DISPLAY_ON;

		xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
							 &oled,		   /* 发送的消息内容 */
							 100);		   /* 等待时间 100 Tick */
		if (xReturn != pdPASS)
			dgb_printf_safe("[soft_timer_callback id=%d] xQueueSend oled error code is %d\r\n", timer_id, xReturn);
	}

	/* 进入临界区，FreeRTOS同时会关闭中断（UCOS的OS_CRITICAL_ENTER则停止任务调度） */
	taskENTER_CRITICAL();

#if IWDG_ENABLE
	/* 喂狗，刷新自身计数值 */
	IWDG_ReloadCounter();
#endif
	/* 统计系统无操作计数值自加1 */
	g_system_no_opreation_cnt++;

	/* 退出临界区，FreeRTOS同时会打开中断（UCOS的OS_CRITICAL_EXTI则恢复任务调度） */
	taskEXIT_CRITICAL();
}

/*ESP8266WIFI模块*/
void app_task_esp8266(void *pvParameters)
{
	uint8_t buf[MAX_DATA_SIZE];
	uint8_t led_sta = 0;
	beep_t beep;
	uint8_t motor;
	oled_t oled;
	flash_t flash;
	BaseType_t xReturn = pdFALSE;
	uint32_t i;
	uint32_t j;
	uint32_t ret = 0;
	uint8_t jsonString[MAX_DATA_SIZE] = {0};

	dgb_printf_safe("[app_task_esp8266] create success\r\n");

	while (esp8266_mqtt_init())
	{
		dgb_printf_safe("[app_task_esp8266] esp8266_mqtt_init ...");

		delay_ms(1000);
	}

	// 蜂鸣器嘀两声，D1灯闪烁两次，示意连接成功
	/* 嘀一声示意:第一次 */
	beep.sta = 1;
	beep.duration = 50;
	xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
						 &beep,		   /* 发送的消息内容 */
						 100);		   /* 等待时间 100 Tick */
	if (xReturn != pdPASS)
		dgb_printf_safe("[app_task_esp8266] xQueueSend beep error code is %d\r\n", xReturn);
	/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
	xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
	if (xReturn != pdPASS)
		dgb_printf_safe("[app_task_esp8266] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
	delay_ms(100);
	/* 嘀一声示意:第二次 */
	beep.sta = 1;
	beep.duration = 50;
	xReturn = xQueueSend(g_queue_beep, /* 消息队列的句柄 */
						 &beep,		   /* 发送的消息内容 */
						 100);		   /* 等待时间 100 Tick */
	if (xReturn != pdPASS)
		dgb_printf_safe("[app_task_esp8266] xQueueSend beep error code is %d\r\n", xReturn);
	/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
	xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
	if (xReturn != pdPASS)
		dgb_printf_safe("[app_task_esp8266] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);

	dgb_printf_safe("[app_task_esp8266] esp8266 connect aliyun with mqtt success\r\n");

	vTaskResume(app_task_mqtt_handle); // 唤醒MQTT任务，允许跟阿里云进行通信

	g_esp8266_init = 1; // 标志esp8266初始化完成

	for (;;)
	{
		/*等待阿里云发送数据包*/
		xReturn = xQueueReceive(g_queue_esp8266, /* 消息队列的句柄 */
								buf,			 /* 得到的消息内容 */
								portMAX_DELAY);	 /* 等待时间一直等 */

		/* 清零统计系统无操作计数值 */
		taskENTER_CRITICAL();
		g_system_no_opreation_cnt = 0;
		taskEXIT_CRITICAL();

		if (xReturn != pdPASS)
		{
			dgb_printf_safe("[app_task_esp8266] xQueueReceive error code is %d\r\n", xReturn);
			continue;
		}
		i = 0;
		while (buf[i] != '{') // 忽略垃圾数据头
		{
			/* code */
			i++;
			if (i >= MAX_DATA_SIZE)
				break;
		}
		for (j = 0, i; i < sizeof(buf); i++, j++)
		{
			sprintf(&jsonString[j], "%c", buf[i]);
		}

		jsonString[j] = '\0';
		// 简单方法解析
		if (strstr(jsonString, "\"switch_led_1\":0") != NULL)
		{
			dgb_printf_safe("[app_task_esp8266] switch_led_1:0\r\n");
			led_sta = LED1_OFF;
			xReturn = xQueueSend(g_queue_led, &led_sta, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_esp8266] xQueueSend led error code is %d\r\n", xReturn);
		}
		if (strstr(jsonString, "\"switch_led_1\":1") != NULL)
		{
			dgb_printf_safe("[app_task_esp8266] switch_led_1:1\r\n");
			led_sta = LED1_ON;
			xReturn = xQueueSend(g_queue_led, &led_sta, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_esp8266] xQueueSend led error code is %d\r\n", xReturn);
		}
		if (strstr(jsonString, "\"switch_led_2\":0") != NULL)
		{
			dgb_printf_safe("[app_task_esp8266] switch_led_2:0\r\n");
			led_sta = LED2_OFF;
			xReturn = xQueueSend(g_queue_led, &led_sta, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_esp8266] xQueueSend led error code is %d\r\n", xReturn);
		}
		if (strstr(jsonString, "\"switch_led_2\":1") != NULL)
		{
			dgb_printf_safe("[app_task_esp8266] switch_led_2:1\r\n");
			led_sta = LED2_ON;
			xReturn = xQueueSend(g_queue_led, &led_sta, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_esp8266] xQueueSend led error code is %d\r\n", xReturn);
		}
		if (strstr(jsonString, "\"switch_led_3\":0") != NULL)
		{
			dgb_printf_safe("[app_task_esp8266] switch_led_3:0\r\n");
			led_sta = LED3_OFF;
			xReturn = xQueueSend(g_queue_led, &led_sta, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_esp8266] xQueueSend led error code is %d\r\n", xReturn);
		}
		if (strstr(jsonString, "\"switch_led_3\":1") != NULL)
		{
			dgb_printf_safe("[app_task_esp8266] switch_led_3:1\r\n");
			led_sta = LED3_ON;
			xReturn = xQueueSend(g_queue_led, &led_sta, 100);
			if (xReturn != pdPASS)
				dgb_printf_safe("[app_task_esp8266] xQueueSend led error code is %d\r\n", xReturn);
		}
		if (g_unlock_what == FLAG_UNLOCK_OK)
		{
			if (strstr(jsonString, "\"lock_1\":0") != NULL)
			{
				dgb_printf_safe("[app_task_esp8266] lock_1:0\r\n");
				g_pass_man_what = FLAG_PASS_MAN_AUTH; // 开始密码验证
				g_unlock_what = FLAG_UNLOCK_NO;
				// 显示密码输入界面
				oled.ctrl = OLED_CTRL_CLEAR;
				xReturn = xQueueSend(g_queue_oled, /* 消息队列的句柄 */
									 &oled,		   /* 发送的消息内容 */
									 100);		   /* 等待时间 100 Tick */
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_esp8266] xQueueSend oled string error code is %d\r\n", xReturn);
				motor = MOTOR_DOUBLE_REV;
				xReturn = xQueueSend(g_queue_motor, &motor, 100);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_esp8266] xQueueSend motor error code is %d\r\n", xReturn);
				/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
				xReturn = xSemaphoreTake(g_sem_motor, portMAX_DELAY);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_esp8266] xSemaphoreTake g_sem_motor error code is %d\r\n", xReturn);
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
					dgb_printf_safe("[app_task_esp8266] xQueueSend oled picture error code is %d\r\n", xReturn);
				g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // 恢复默认解锁模式
			}
		}
		if (g_unlock_what == FLAG_UNLOCK_NO)
		{
			if (strstr(jsonString, "\"lock_1\":1") != NULL)
			{
				dgb_printf_safe("[app_task_esp8266] lock_1:1\r\n");
				g_unlock_what = FLAG_UNLOCK_OK;			  // 设备解锁
				g_pass_man_what = FLAG_PASS_MAN_NONE;	  // 解锁成功关闭键盘解锁蓝牙解锁，关闭密码验证
				g_pass_unlock_mode = MODE_OPEN_LOCK_MQTT; // 设置为MQTT远程解锁模式

				motor = MOTOR_DOUBLE_POS;
				xReturn = xQueueSend(g_queue_motor, &motor, 100);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_fpm383] xQueueSend motor error code is %d\r\n", xReturn);
				/* [可选]阻塞等待信号量，用于确保任务完成对beep的控制 */
				xReturn = xSemaphoreTake(g_sem_motor, portMAX_DELAY);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_fpm383] xSemaphoreTake g_sem_motor error code is %d\r\n", xReturn);

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
					dgb_printf_safe("[app_task_fpm383] xQueueSend oled picture error code is %d\r\n", xReturn);
				//  发送flash消息记录解锁日志
				flash.mode = g_pass_unlock_mode; // 获取当前解锁模式，存入flash
				xReturn = xQueueSend(g_queue_flash, &flash, 100);
				if (xReturn != pdPASS)
					dgb_printf_safe("[app_task_fpm383] xQueueSend flash error code is %d\r\n", xReturn);
			}
		}
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
