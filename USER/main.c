
#include "includes.h"

/*��������һ��Ӳ��һ������������*/
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

/*���������*/
static void app_task_init(void *pvParameters);		   /* 	�豸��ʼ������ */
static void app_task_led(void *pvParameters);		   /* 	led���� ����LED��Ϣ���е���Ϣ��������Ӧ*/
static void app_task_led_breath(void *pvParameters);   /*	���������񣬴�Ž�����¼����Ϣ*/
static void app_task_beep(void *pvParameters);		   /* 	beep���� ���շ�������Ϣ���е���Ϣ��������Ӧ��Ӧ*/
static void app_task_key(void *pvParameters);		   /* 	key���� */
static void app_task_keyboard(void *pvParameters);	   /* 	keyboard���� */
static void app_task_rtc(void *pvParameters);		   /*	rtcʵʱʱ������*/
static void app_task_dht(void *pvParameters);		   /* 	dth11���� ��ȡ��ʪ����Ϣ�����͵�oled��ʾ����Ϣ����*/
static void app_task_sr04(void *pvParameters);		   /*	sr04���񣬻�ȡ������Ϣ�����͵�oled��ʾ����Ϣ����*/
static void app_task_oled(void *pvParameters);		   /*	oled���񣬽��ղ�����g_queue_oled���и�����Ϣ����ʾ����*/
static void app_task_flash(void *pvParameters);		   /*	flash���񣬴�Ž�����¼����Ϣ*/
static void app_task_usart(void *pvParameters);		   /*	usart���񣬽��մ��ڶ��е���Ϣ������*/
static void app_task_mfrc522real(void *pvParameters);  /*	rfid����������*/
static void app_task_mfrc522admin(void *pvParameters); /*	rfid����������*/
static void app_task_password_man(void *pvParameters); /*	password management���񣬽��վ�����̶��е���Ϣ������*/
static void app_task_fpm383(void *pvParameters);	   /*	fpm383����ָ��������*/
/*�������ź������*/
SemaphoreHandle_t g_sem_led;  // ͬ��led�������������
SemaphoreHandle_t g_sem_beep; // ͬ��beep�������������

/*�������ź������*/
SemaphoreHandle_t g_mutex_oled;

/*�¼���־������32bitλ��־�飩���ڹ����������־λ*/
EventGroupHandle_t g_event_group;

/*��Ϣ���о��*/
QueueHandle_t g_queue_usart; // ���մ��ڷ���������Ϣ
QueueHandle_t g_queue_led;
QueueHandle_t g_queue_beep;
QueueHandle_t g_queue_oled;
QueueHandle_t g_queue_flash;
QueueHandle_t g_queue_keyboard;

/*ȫ�ֱ���*/
volatile uint32_t g_rtc_get_what = FLAG_RTC_GET_NONE;
volatile uint32_t g_dht_get_what = FLAG_DHT_GET_NONE;
volatile uint32_t g_unlock_what = FLAG_UNLOCK_NO;
volatile uint32_t g_pass_man_what = FLAG_PASS_MAN_NONE;					 // ��������־
volatile uint32_t g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD;			 // �������ģʽ��Ĭ�ϼ��̽��������ݲ�ͬ������ʽ���Զ����ĸñ�־λ
const char pass_auth_default[PASS_LEN] = {'8', '8', '8', '8', '8', '8'}; // Ĭ������

int main(void)
{
	/* ����ϵͳ�ж����ȼ�����ֻ��ѡ���4�� */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ 168000000/1000 */
	SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);

	//	/* ��ʼ������1 */
	//	usart1_init(115200);
	/*��ʼ������3*/
	usart3_init(9600);

	/* ���� app_task_init���� */
	xTaskCreate((TaskFunction_t)app_task_init,			/* ������ں��� */
				(const char *)"app_task_init",			/* �������� */
				(uint16_t)512,							/* ����ջ��С */
				(void *)NULL,							/* ������ں������� */
				(UBaseType_t)5,							/* ��������ȼ� */
				(TaskHandle_t *)&app_task_init_handle); /* ������ƿ�ָ�� */

	/* ����������� */
	vTaskStartScheduler();

	while (1)
		;
}

/* �����б� */
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

	/* �����õ������� */
	taskDISABLE_INTERRUPTS();
	i = 0;
	while (task_tbl[i].pxTaskCode)
	{
		xTaskCreate(task_tbl[i].pxTaskCode,		/* ������ں��� */
					task_tbl[i].pcName,			/* �������� */
					task_tbl[i].usStackDepth,	/* ����ջ��С */
					task_tbl[i].pvParameters,	/* ������ں������� */
					task_tbl[i].uxPriority,		/* ��������ȼ� */
					task_tbl[i].pxCreatedTask); /* ������ƿ�ָ�� */
		i++;
	}
	taskENABLE_INTERRUPTS();

	/*�����������ź��������ֵΪ10����ֵΪ0*/
	g_sem_led = xSemaphoreCreateCounting(10, 0);
	g_sem_beep = xSemaphoreCreateCounting(10, 0);
	/*�����¼���־��*/
	g_event_group = xEventGroupCreate();
	/*������Ϣ����,Ҫע����Ϣ���е������Ƿ񴴽���ȷ*/
	g_queue_led = xQueueCreate(QUEUE_LED_LEN, sizeof(uint8_t));			  // ����ledͨ�ŵ���Ϣ����
	g_queue_beep = xQueueCreate(QUEUE_BEEP_LEN, sizeof(beep_t));		  // ����beepͨ�ŵ���Ϣ����
	g_queue_oled = xQueueCreate(QUEUE_OLED_LEN, sizeof(oled_t));		  // ����oledͨ�ŵ���Ϣ����
	g_queue_usart = xQueueCreate(QUEUE_USART_LEN, sizeof(usart_t));		  // ����usartͨ�ŵ���Ϣ����
	g_queue_flash = xQueueCreate(QUEUE_FLASH_LEN, sizeof(flash_t));		  // ����flashͨ�ŵ���Ϣ����
	g_queue_keyboard = xQueueCreate(QUEUE_KEYBOARD_LEN, sizeof(uint8_t)); // ����keyboardͨ�ŵ���Ϣ����

	/*ʵ���˸�λ������flash��ʵʱʱ�ӵ�ǰ����ʱ��*/
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 4455)
	{
		rtc_init();	  // ʵʱʱ��RTC��ʼ�������������ں�ʱ��
		flash_init(); // �������4
		RTC_WriteBackupRegister(RTC_BKP_DR0, 4455);
	}
	else
	{
		rtc_resume_init(); // ʵʱʱ��RTC��ʼ�������������ں�ʱ��
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
	/* ��ʾlogo */
	OLED_DrawBMP(0, 0, 128, 8, (uint8_t *)pic_logo);
	vTaskDelay(3000);
	OLED_Clear();
	OLED_DrawBMP(32, 0, 96, 8, (uint8_t *)pic_lock_icon);

	/* ɾ���������� */
	vTaskDelete(NULL);
	printf("[app_task_init] nerver run here\r\n");
}

/*led������*/
static void app_task_led(void *pvParameters)
{
	uint8_t led_sta = 0; // led״̬��־λ
	BaseType_t xReturn = pdFALSE;
	printf("app_task_led is running ...\r\n");
	for (;;) // while(1)
	{
		xReturn = xQueueReceive(g_queue_led,	/* ��Ϣ���еľ�� */
								&led_sta,		/* �õ�����Ϣ���� */
								portMAX_DELAY); /* �ȴ�ʱ��һֱ�� */
		// ������Ϣ���д���������Ϣ
		if (xReturn != pdPASS)
			continue;
		/* ��⵽����LED1 ����00010001(0x11)����00010000(0x10)	*/
		if (led_sta & 0x10) // ����֮������ͨ����λ��ʵ������һ���ֽ�(8bit)������led�Ƶ�����ǰ4bitѡ��ƣ����1bit��������
		{
			if (led_sta & 0x01)
				PFout(9) = 0;
			else
				PFout(9) = 1;
		}
		/* ��⵽����LED2 ����00100001(0x21)����00100000(0x20)	*/
		if (led_sta & 0x20)
		{
			if (led_sta & 0x01)
				PFout(10) = 0;
			else
				PFout(10) = 1;
		}
		/* ��⵽����LED3 01000001(0x41)����01000000(0x40)	*/
		if (led_sta & 0x40)
		{
			if (led_sta & 0x01)
				PEout(13) = 0;
			else
				PEout(13) = 1;
		}
		/* ��⵽����LED4 10000001(0x81)����10000000(0x80)	*/
		if (led_sta & 0x80)
		{
			if (led_sta & 0x01)
				PEout(14) = 0;
			else
				PEout(14) = 1;
		}
		/* �ͷ��ź��������߶Է�����ǰled���������Ѿ���� */
		xSemaphoreGive(g_sem_led);
	}
}
/*������ģ��*/
void app_task_led_breath(void *pvParameters)
{
	int8_t brightness = 0;

	printf("[app_task_led_breath] create success and suspend self\r\n");

	vTaskSuspend(NULL); // �ȹ��������

	printf("[app_task_led_breath] resume success\r\n");

	for (;;)
	{
		/* ���� */
		for (brightness = 0; brightness <= 100; brightness++)
		{
			led_breath_brightness(brightness);
			vTaskDelay(20);
		}

		/* ���� */
		for (brightness = 100; brightness >= 0; brightness--)
		{
			led_breath_brightness(brightness);
			vTaskDelay(20);
		}
	}
}
/*������ģ��*/
static void app_task_beep(void *pvParameters)
{
	beep_t beep;
	BaseType_t xReturn;
	printf("app_task_beep is running ...\r\n");
	for (;;) // while(1)
	{
		xReturn = xQueueReceive(g_queue_beep,	/* ��Ϣ���еľ�� */
								&beep,			/* �õ�����Ϣ���� */
								portMAX_DELAY); /* �ȴ�ʱ��һֱ�� */
		// ������Ϣ���д���������Ϣ
		if (xReturn != pdPASS)
			continue;
		// printf("[app_task_beep] beep.sta=%d beep.duration=%d\r\n", beep.sta, beep.duration);
		// ���������Ƿ���Ҫ��������
		if (beep.duration)
		{
			BEEP(beep.sta);
			while (beep.duration--)
				vTaskDelay(1);
			/* ������״̬��ת */
			beep.sta ^= 1;
		}
		BEEP(beep.sta);
		/* �ͷ��ź��������߶Է�����ǰbeep���������Ѿ���� */
		xSemaphoreGive(g_sem_beep);
	}
}
/*����ģ��*/
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
		// �ȴ��¼����е���Ӧ�¼�λ����ͬ��
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_KEYALL_DOWN,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
		memset(&oled, 0, sizeof(oled));
		// ���ݷ���ֵ�ж��ĸ�����������
		if (EventValue & EVENT_GROUP_KEY1_DOWN) // ����1
		{
			// ��ֹEXTI0�����ж�
			NVIC_DisableIRQ(EXTI0_IRQn);
			// ��ʱ����
			vTaskDelay(50);
			// ȷ���ǰ���
			if (PAin(0) == 0)
			{
				printf("[app_task_key] S1 Press\r\n");
				vTaskSuspend(app_task_rtc_handle);	// ����ʵʱʱ������
				vTaskSuspend(app_task_sr04_handle); // ��������ģ��
				g_rtc_get_what = FLAG_RTC_GET_NONE; // ����ʾʱ����Ϣ
				flash_read(10);
				// ����ע��ָ���¼�
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_ADD);
			}
			// �ȴ������ͷ�
			while (PAin(0) == 0)
				vTaskDelay(1);
			// ����EXTI0�����ж�
			NVIC_EnableIRQ(EXTI0_IRQn);
		}
		if (EventValue & EVENT_GROUP_KEY2_DOWN) // ����2
		{
			// ��ֹEXTI2�����ж�
			NVIC_DisableIRQ(EXTI2_IRQn);
			// ��ʱ����
			vTaskDelay(50);
			if (PEin(2) == 0)
			{
				printf("[app_task_key] S2 Press\r\n");
				vTaskSuspend(app_task_rtc_handle);	// ����ʵʱʱ������
				vTaskSuspend(app_task_sr04_handle); // ��������ģ��
				g_rtc_get_what = FLAG_RTC_GET_NONE; // 	����ʾʱ����Ϣ

				// 	// �����¼���־λ0x20,�����¶ȼ�¼����
				// xEventGroupSetBits(g_event_group, EVENT_GROUP_DHT);
				// ������ָ֤���¼�
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_AUTH);
			}
			// �ȴ������ͷ�
			while (PEin(2) == 0)
				vTaskDelay(1);
			// ����EXTI2�����ж�
			NVIC_EnableIRQ(EXTI2_IRQn);
		}
		if (EventValue & EVENT_GROUP_KEY3_DOWN) // ����3
		{
			// ��ֹEXTI3�����ж�
			NVIC_DisableIRQ(EXTI3_IRQn);
			// ��ʱ����
			vTaskDelay(50);
			if (PEin(3) == 0)
			{
				printf("[app_task_key] S1 Press\r\n");
				g_rtc_get_what = FLAG_RTC_GET_NONE;
				// ������ʾָ�������¼�
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_SHOW);
			}
			// �ȴ������ͷ�
			while (PEin(3) == 0)
				vTaskDelay(1);
			// ����EXTI3�����ж�
			NVIC_EnableIRQ(EXTI3_IRQn);
		}

		if (EventValue & EVENT_GROUP_KEY4_DOWN) // ����4
		{
			// ��ֹEXTI4�����ж�
			NVIC_DisableIRQ(EXTI4_IRQn);
			// ��ʱ����
			vTaskDelay(50);
			if (PEin(4) == 0)
			{
				printf("[app_task_key] S4 Press\r\n");
				// ����ɾ��ָ���¼�
				xEventGroupSetBits(g_event_group, EVENT_GROUP_FPM_DELETE);
				vTaskResume(app_task_rtc_handle);	// �ָ�rtc����
				g_rtc_get_what = FLAG_RTC_GET_TIME; // ��ȡʱ��
				// g_rtc_get_what = FLAG_RTC_GET_DATE; // ��ȡ����

				oled.ctrl = OLED_CTRL_CLEAR; /* oled���� */
				xReturn = xQueueSend(g_queue_oled, &oled, 100);
				if (xReturn != pdPASS)
					printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			}

			// �ȴ������ͷ�
			while (PEin(4) == 0)
				vTaskDelay(1);

			NVIC_EnableIRQ(EXTI4_IRQn); // ����EXTI4�����ж�
		}
	}
}
/*�������ģ��*/
void app_task_keyboard(void *pvParameters)
{
	char key_val = 'N';
	beep_t beep;
	BaseType_t xReturn;
	printf("[app_task_kbd] create success\r\n");

	while (1)
	{
		/* ��ȡ������̰���ֵ */
		key_val = kbd_read();

		if (key_val != 'N')
		{
			printf("[app_task_kbd] kbd press %c \r\n", key_val);

			xReturn = xQueueSend(g_queue_keyboard, &key_val, 100);
			if (xReturn != pdPASS)
				printf("[app_task_kbd] xQueueSend kbd error code is %d\r\n", xReturn);

			/* ��һ��ʾ�� */
			beep.sta = 1;
			beep.duration = 10;
			xReturn = xQueueSend(g_queue_beep, &beep, 100);
			if (xReturn != pdPASS)
				printf("[app_task_kbd] xQueueSend beep error code is %d\r\n", xReturn);

			/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
			xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
			if (xReturn != pdPASS)
				printf("[app_task_system_reset] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
		}
	}
}
/*rtcʵʱʱ��ģ��*/
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
		/* �ȴ��¼����е���Ӧ�¼�λ����ͬ�� */
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_RTC_WAKEUP | EVENT_GROUP_RTC_ALARM,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);

		if (EventValue & EVENT_GROUP_RTC_WAKEUP) // RTC�¼�������
		{
			memset(buf, 0, sizeof buf);
			if (g_rtc_get_what == FLAG_RTC_GET_TIME)
			{
				/* RTC_GetTime����ȡʱ�� */
				RTC_GetTime(RTC_Format_BCD, &RTC_TimeStructure);
				memset(buf, 0, sizeof buf);
				/* ��ʽ���ַ��� */
				sprintf((char *)buf, "%02x:%02x:%02x", RTC_TimeStructure.RTC_Hours,
						RTC_TimeStructure.RTC_Minutes,
						RTC_TimeStructure.RTC_Seconds);

				oled.x = 32;
			}
			if (g_rtc_get_what == FLAG_RTC_GET_DATE)
			{
				/* RTC_GetTime����ȡ���� */
				RTC_GetDate(RTC_Format_BCD, &RTC_DateStructure);

				memset(buf, 0, sizeof buf);
				/* ��ʽ���ַ��� */
				sprintf((char *)buf, "20%02x-%02x-%02x-%d", RTC_DateStructure.RTC_Year,
						RTC_DateStructure.RTC_Month,
						RTC_DateStructure.RTC_Date,
						RTC_DateStructure.RTC_WeekDay);
				oled.x = 15;
			}
			/* oled��ʾʱ�� */
			oled.ctrl = OLED_CTRL_SHOW_STRING;
			oled.y = 4;
			oled.str = buf; // �ַ����������׵�ַ
			oled.font_size = 16;

			xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
								 &oled,		   /* ���͵���Ϣ���� */
								 100);		   /* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
		}
		if (EventValue & EVENT_GROUP_RTC_ALARM) // ���ӱ������¼�
		{
			printf("���ӻ�����\r\n");
		}
	}
}
/*��ʪ��ģ��*/
static void app_task_dht(void *pvParameters)
{
	static uint32_t cnt = 0; // ���Ƽ�¼���¶�ֻ��10��
	uint8_t dht11_buf[5];	 // �洢��ʪ��ģ���ȡ��������
	int32_t dht11_read_ret;	 // ��ʪ�ȶ�ȡ��������ֵ
	uint8_t g_temp_buf[32];	 // ��ʽ����¼��ǰ��ʪ��
	oled_t oled;
	flash_t flash;
	BaseType_t xReturn;
	EventBits_t EventValue;

	for (;;) // while(1)
	{
		// �ȴ��¼����е���Ӧ�¼�λ����ͬ��
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
				// �����ݷ�װ��flash�洢�ṹ���ʽ�����͵�flash����
				flash.mode = MODE_OPEN_LOCK_DHT;
				sprintf((char *)flash.databuf, "Temp=%d.%d Humi=%d.%d", dht11_buf[2], dht11_buf[3], dht11_buf[0], dht11_buf[1]);
				xReturn = xQueueSend(g_queue_flash, /* ��Ϣ���еľ�� */
									 &flash,		/* ���͵���Ϣ���� */
									 100);			/* �ȴ�ʱ�� 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_dht] xQueueSend flash string error code is %d\r\n", xReturn);

				// �����ݷ�װ��oled��ʽ���͸�oled����
				sprintf((char *)g_temp_buf, "Temp=%d.%d", dht11_buf[2], dht11_buf[3]);
				// ���͸�g_queue_oled
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.x = 32;
				oled.y = 2;
				oled.str = g_temp_buf; // �ַ����������׵�ַ
				oled.font_size = 16;
				xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
									 &oled,		   /* ���͵���Ϣ���� */
									 100);		   /* �ȴ�ʱ�� 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
				vTaskDelay(500); // ������ʱ��������ݻḲ��ǰ���
				memset(&oled, 0, sizeof(oled));
				sprintf((char *)g_temp_buf, "Humi=%d.%d", dht11_buf[0], dht11_buf[1]);
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.x = 32;
				oled.y = 6;
				oled.str = g_temp_buf; // �ַ����������׵�ַ
				oled.font_size = 16;
				xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
									 &oled,		   /* ���͵���Ϣ���� */
									 100);		   /* �ȴ�ʱ�� 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_dht] xQueueSend oled string error code is %d\r\n", xReturn);
			}
			else
			{
				printf("dht11 read error code is %d\r\n", dht11_read_ret);
			}
		}
		vTaskResume(app_task_sr04_handle); // �ָ�������ģ��
	}
}
/*������ģ��*/
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
			xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
								 &oled,		   /* ���͵���Ϣ���� */
								 100);		   /* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			break;
		case 2:
			// Ϣ��
			oled.ctrl = OLED_CTRL_DISPLAY_OFF;
			xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
								 &oled,		   /* ���͵���Ϣ���� */
								 100);		   /* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			break;
		case 3:
			// ����
			oled.ctrl = OLED_CTRL_DISPLAY_ON;
			xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
								 &oled,		   /* ���͵���Ϣ���� */
								 100);		   /* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_rtc] xQueueSend oled string error code is %d\r\n", xReturn);
			break;
		default:
			break;
		}
		vTaskDelay(3000);
	}
}
/*oledģ��*/
static void app_task_oled(void *pvParameters)
{
	oled_t oled;
	BaseType_t xReturn = pdFALSE;
	printf("[app_task_oled] create success\r\n");
	memset(&oled, 0, sizeof(oled));
	for (;;)
	{
		xReturn = xQueueReceive(g_queue_oled,	/* ��Ϣ���еľ�� */
								&oled,			/* �õ�����Ϣ���� */
								portMAX_DELAY); /* �ȴ�ʱ��һֱ�� */
		if (xReturn != pdPASS)
			continue;

		switch (oled.ctrl)
		{
		case OLED_CTRL_DISPLAY_ON:
		{
			/* ���� */
			OLED_Display_On();
			/* ������ֹͣ���� */
			vTaskSuspend(app_task_led_breath_handle);
			led_breath_stop();
		}
		break;

		case OLED_CTRL_DISPLAY_OFF:
		{
			/* ���� */
			OLED_Display_Off();
			/* �����ƿ�ʼ���� */
			led_breath_run();
			vTaskResume(app_task_led_breath_handle);
		}
		break;

		case OLED_CTRL_CLEAR:
		{
			/* ���� */
			OLED_Clear();
		}
		break;

		case OLED_CTRL_SHOW_STRING:
		{
			/* ��ʾ�ַ��� */
			OLED_ShowString(oled.x,
							oled.y,
							oled.str,
							oled.font_size);
		}
		break;

		case OLED_CTRL_SHOW_CHINESE:
		{
			/* ��ʾ���� */
			OLED_ShowCHinese(oled.x,
							 oled.y,
							 oled.chinese);
		}
		break;

		case OLED_CTRL_SHOW_PICTURE:
		{
			/* ��ʾͼƬ */
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
		xReturn = xQueueReceive(g_queue_flash,	  /* ��Ϣ���еľ�� */
								&flash_write_buf, /* ���յ���Ϣ���� */
								portMAX_DELAY);	  /* �ȴ�ʱ��һֱ�� */
		if (xReturn != pdPASS)
			continue;

		printf("[app_task_flash] wait get data\r\n");

		// ��ȡ��ǰ����д����ʶ
		flash_write_buf.offset = flash_read_offset((uint32_t *)(0x08010000));
		if (flash_write_buf.offset >= 10)
		{
			flash_init();				// �������
			flash_write_buf.offset = 0; // ��0��ʼ����д��
		}

		RTC_GetDate(RTC_Format_BCD, &RTC_Date);
		RTC_GetTime(RTC_Format_BCD, &RTC_Time);

		printf("��ǰʱ��:20%02x/%02x/%02x %02x:%02x:%02x %d\n",
			   RTC_Date.RTC_Year,
			   RTC_Date.RTC_Month,
			   RTC_Date.RTC_Date,
			   RTC_Time.RTC_Hours,
			   RTC_Time.RTC_Minutes,
			   RTC_Time.RTC_Seconds,
			   flash_write_buf.mode);

		// ��¼��ÿ��д�����ݵ�����ʱ��
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

/*����ģ��*/
static void app_task_usart(void *pvParameters)
{
	uint8_t years;	 // ��
	uint8_t month;	 // ��
	uint8_t week;	 // ��
	uint8_t date;	 // ���ӣ���
	uint8_t hours;	 // ���ӣ�ʱ
	uint8_t minutes; // ���ӣ���
	uint8_t seconds; // ���ӣ���
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
		// ��Ϣ����Ϊ�յ�ʱ�������������Ҫ���¼���־�鴥������
		xReturn = xQueueReceive(g_queue_usart,	/* ��Ϣ���еľ�� */
								&usart_packet,	/* �õ�����Ϣ���� */
								portMAX_DELAY); /* �ȴ�ʱ��һֱ�� */
		if (xReturn != pdPASS)
		{
			printf("[app_task_usart] xQueueReceive usart_packet error code is %d\r\n", xReturn);
			continue;
		}
		// ������Ϣ
		printf("[app_task_usart] recv data:%s\r\n", usart_packet.rx_buf);

		// ������ʪ�����ݼ�¼ģʽ
		if (strstr((char *)usart_packet.rx_buf, "temp?#") || strstr((char *)usart_packet.rx_buf, "start"))
		{
		}
		if (strstr((char *)usart_packet.rx_buf, "ALARM SET")) // �������ӣ���-ʱ-��-��
		{
			// ��ȡ ��-ʱ-��-��
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
			rtc_alarm_init(hours, minutes, seconds, date); // ����
			printf("[app_task_usart] rtc set alarm ok\r\n");
		}

		/* �������� */
		if (strstr((char *)usart_packet.rx_buf, "DATE SET"))
		{
			/* �ԵȺŷָ��ַ��� */
			p = strtok((char *)usart_packet.rx_buf, "-");
			// ��ȡ��
			// 2021-2000=21
			years = to_hex(atoi(strtok(NULL, "-")) - 2000);
			RTC_DateStructure.RTC_Year = years;
			// ��ȡ��
			month = to_hex(atoi(strtok(NULL, "-")));
			RTC_DateStructure.RTC_Month = month;
			// ��ȡ��
			date = to_hex(atoi(strtok(NULL, "-")));
			RTC_DateStructure.RTC_Date = date;
			// ��ȡ����
			week = to_hex(atoi(strtok(NULL, "-")));
			RTC_DateStructure.RTC_WeekDay = week;
			// ����
			RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
			printf("[app_task_usart] rtc set date ok\r\n");
		}
		/* ����ʱ�� */
		else if (strstr((char *)usart_packet.rx_buf, "TIME SET"))
		{
			/* �ԵȺŷָ��ַ��� */
			p = strtok((char *)usart_packet.rx_buf, "-");
			/* ��ȡʱ */
			p = strtok(NULL, "-");
			i = atoi(p);
			/* ͨ��ʱ���ж���AM����PM */
			if (i < 12)
				RTC_TimeStructure.RTC_H12 = RTC_H12_AM;
			else
				RTC_TimeStructure.RTC_H12 = RTC_H12_PM;

			/* ת��ΪBCD���� */
			i = (i / 10) * 16 + i % 10;
			RTC_TimeStructure.RTC_Hours = i;
			/* ��ȡ�� */
			minutes = to_hex(atoi(strtok(NULL, "-")));
			RTC_TimeStructure.RTC_Minutes = minutes;
			/* ��ȡ�� */
			seconds = to_hex(atoi(strtok(NULL, "#")));
			RTC_TimeStructure.RTC_Seconds = seconds;

			/* ����RTCʱ�� */
			RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);

			printf("[app_task_usart] rtc set time ok\r\n");
		}
		else if (strstr((char *)usart_packet.rx_buf, "PASS UNLOCK") && (g_pass_man_what == FLAG_PASS_MAN_AUTH)) // ��֤����
		{
			p = strtok((char *)usart_packet.rx_buf, "PASS UNLOCK");
			printf("[app_task_usart] recv pass :%s\r\n", p);
			g_pass_unlock_mode = MODE_OPEN_LOCK_BLUE; // ��������ģʽ
			for (i = 0; i < 8; i++)
			{
				key_val = p[i];
				xReturn = xQueueSend(g_queue_keyboard, /* ��Ϣ���еľ�� */
									 &key_val,		   /* ���͵���Ϣ���� */
									 100);			   /* �ȴ�ʱ�� 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_kbd] xQueueSend kbd error code is %d\r\n", xReturn);
			}
		}
	}
}

/*�������ģ��*/
void app_task_password_man(void *pvParameters)
{
	char pass_auth_eeprom[PASS_LEN] = {0}; // �洢��e2prom���������

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
		// �ȴ����������Ϣ������Ϣ
		xReturn = xQueueReceive(g_queue_keyboard, &key_val, portMAX_DELAY);
		if (xReturn != pdPASS)
			continue;

		if (key_val == 'C')
		{
			printf("password reset\r\n");
			g_unlock_what = FLAG_UNLOCK_NO;		  // �豸����
			g_pass_man_what = FLAG_PASS_MAN_AUTH; // ��ʼ������֤
			/*�ָ���ֵ*/
			x = x_start;
			y = y_start;
			key_cnt = 0;
			memset(key_buf, 0, sizeof key_buf);
			// ��ʾ�����������
			oled.ctrl = OLED_CTRL_CLEAR;
			xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
								 &oled,		   /* ���͵���Ϣ���� */
								 100);		   /* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
			oled.ctrl = OLED_CTRL_SHOW_STRING;
			oled.x = x;
			oled.y = y;
			oled.font_size = 16;
			oled.str = "------";
			xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
								 &oled,		   /* ���͵���Ϣ���� */
								 100);		   /* �ȴ�ʱ�� 100 Tick */
			if (xReturn != pdPASS)
				printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

			continue;
		}
		if (g_pass_man_what == FLAG_PASS_MAN_AUTH) // ����������֤
		{
			/* û�����룬��Чɾ������ */
			if (key_val == '*' && key_cnt == 0)
				continue;

			/* ɾ����һ������ */
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
				xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
									 &oled,		   /* ���͵���Ϣ���� */
									 100);		   /* �ȴ�ʱ�� 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
				continue;
			}

			if (key_cnt < 6)
			{
				/* ��ʾ���� */
				key_buf[key_cnt] = key_val;
				oled.ctrl = OLED_CTRL_SHOW_STRING;
				oled.x = x;
				oled.y = y;
				oled.font_size = 16;
				oled.str = (uint8_t *)&key_buf[key_cnt];
				xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
									 &oled,		   /* ���͵���Ϣ���� */
									 100);		   /* �ȴ�ʱ�� 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

				vTaskDelay(100);
				/* ��ʾ* */
				oled.str = "*";
				xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
									 &oled,		   /* ���͵���Ϣ���� */
									 100);		   /* �ȴ�ʱ�� 100 Tick */
				if (xReturn != pdPASS)
					printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);

				x += 8;
				key_cnt++;
			}
			else if (key_val == '#' && key_cnt >= 6)
			{
				printf("[app_task_pass_man] auth key buf is %6s\r\n", key_buf);

				/* ��ȡeeprom�洢������ */
				// at24c02_read(PASS1_ADDR_IN_EEPROM, (uint8_t *)pass_auth_eeprom, PASS_LEN);

				/* ƥ��Ĭ������ */
				rt = memcmp(pass_auth_default, key_buf, PASS_LEN);
				/* ����ƥ��ɹ� */
				if (rt == 0)
				{
					printf("[app_task_pass_man] password auth success\r\n");
					g_unlock_what = FLAG_UNLOCK_OK;		  // �豸����
					g_pass_man_what = FLAG_PASS_MAN_NONE; // ���ٽ���������֤
					/* ����Ҫ��ʾ��ͼƬ-���飨�ɹ��� */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_unlock_icon;

					xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
										 &oled,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* ��һ��ʾ��:��һ�� */
					beep.sta = 1;
					beep.duration = 20;
					xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
										 &beep,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);
					/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					vTaskDelay(100);
					/* ��һ��ʾ��:�ڶ��� */
					beep.sta = 1;
					beep.duration = 20;
					xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
										 &beep,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);
					/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					/* ��ʱһ�� */
					vTaskDelay(2000);
					/* �ָ���ֵ */
					x = x_start;
					key_cnt = 0;
					memset(key_buf, 0, sizeof key_buf);
					/*��ʾ�˵�����*/
					// ������...
					//  ����flash��Ϣ��¼������־
					flash.mode = g_pass_unlock_mode; // ��ȡ��ǰ����ģʽ�����������������̽���������flash
					xReturn = xQueueSend(g_queue_flash, &flash, 100);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend flash error code is %d\r\n", xReturn);
				}
				else
				{
					printf("[app_task_pass_man] password auth fail\r\n");
					g_pass_man_what = FLAG_PASS_MAN_NONE; // �ص��������棬�ȴ���һ�ν���
					/* ����Ҫ��ʾ��ͼƬ-���飨ʧ�ܣ� */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_error_icon;

					xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
										 &oled,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* ����1��ʾ�� */
					beep.sta = 1;
					beep.duration = 1000;
					xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
										 &beep,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);

					/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);

					/* ��ʱһ�� */
					vTaskDelay(2000);
					/* �ָ���ֵ */
					x = x_start;
					key_cnt = 0;
					memset(key_buf, 0, sizeof key_buf);
					/* ��ʾ�������� */
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.pic = pic_lock_icon;
					xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
										 &oled,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
				}
				g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // �ָ�Ĭ�Ͻ�����ʽ
			}
		}
	}
}
/*ָ��ʶ��ģ��*/
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
		// �ȴ��¼����е���Ӧ�¼�λ����ͬ��
		EventValue = xEventGroupWaitBits((EventGroupHandle_t)g_event_group,
										 (EventBits_t)EVENT_GROUP_FPM_ADD | EVENT_GROUP_FPM_AUTH | EVENT_GROUP_FPM_DELETE | EVENT_GROUP_FPM_SHOW,
										 (BaseType_t)pdTRUE,
										 (BaseType_t)pdFALSE,
										 (TickType_t)portMAX_DELAY);
										 
		if (g_unlock_what == FLAG_UNLOCK_NO) // �豸��������ܽ���
		{
			if (EventValue & EVENT_GROUP_FPM_AUTH) // ��ָ֤���¼�
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				printf("[app_task_fpm383] ִ��ˢָ�Ʋ���,�뽫��ָ�ŵ�ָ��ģ�鴥����Ӧ��\r\n");
				/* ����Ϊ0xFFFF����1:Nƥ�� */
				id = 0xFFFF;
				fmp_error_code = fpm_idenify_auto(&id);
				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					printf("[app_task_fpm383] password auth success the fingerprint is %04d\r\n", id);

					g_unlock_what = FLAG_UNLOCK_OK;			 // �豸����
					g_pass_man_what = FLAG_PASS_MAN_NONE;	 // �����ɹ��رռ��̽�����������
					g_pass_unlock_mode = MODE_OPEN_LOCK_SFM; // ����Ϊָ�ƽ���ģʽ

					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					/* ����Ҫ��ʾ��ͼƬ-���飨�ɹ��� */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_unlock_icon;

					xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
										 &oled,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend oled picture error code is %d\r\n", xReturn);
					//  ����flash��Ϣ��¼������־
					flash.mode = g_pass_unlock_mode; // ��ȡ��ǰ����ģʽ������flash
					xReturn = xQueueSend(g_queue_flash, &flash, 100);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend flash error code is %d\r\n", xReturn);
				}
				else
				{
					fpm_ctrl_led(FPM_LED_RED);
					printf("[app_task_pass_man] password auth fail\r\n");
					/* ����Ҫ��ʾ��ͼƬ-���飨ʧ�ܣ� */
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.pic = pic_error_icon;

					xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
										 &oled,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled picture error code is %d\r\n", xReturn);

					/* ����1��ʾ�� */
					beep.sta = 1;
					beep.duration = 1000;
					xReturn = xQueueSend(g_queue_beep, /* ��Ϣ���еľ�� */
										 &beep,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend beep error code is %d\r\n", xReturn);

					/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
					xReturn = xSemaphoreTake(g_sem_beep, portMAX_DELAY);
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xSemaphoreTake g_sem_beep error code is %d\r\n", xReturn);
					/* ��ʱһ�� */
					vTaskDelay(2000);
					/* ��ʾ�������� */
					oled.ctrl = OLED_CTRL_SHOW_PICTURE;
					oled.x = 32;
					oled.y = 0;
					oled.pic_width = 64;
					oled.pic_height = 8;
					oled.pic = pic_lock_icon;
					xReturn = xQueueSend(g_queue_oled, /* ��Ϣ���еľ�� */
										 &oled,		   /* ���͵���Ϣ���� */
										 100);		   /* �ȴ�ʱ�� 100 Tick */
					if (xReturn != pdPASS)
						printf("[app_task_pass_man] xQueueSend oled string error code is %d\r\n", xReturn);
					g_pass_man_what = FLAG_PASS_MAN_NONE; // �ص��������棬�ȴ���һ�ν���
				}
				g_pass_unlock_mode = MODE_OPEN_LOCK_KEYBOARD; // �ָ�Ĭ�Ͻ�����ʽ

				delay_ms(100);
				fpm_sleep();
				delay_ms(1000);
			}
		}
		if (g_unlock_what == FLAG_UNLOCK_OK) // �豸������ſ��Լ���
		{
			// ���ָ���¼�
			if (EventValue & EVENT_GROUP_FPM_ADD)
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				printf("[app_task_fpm383] ִ�����ָ�Ʋ���,�뽫��ָ�ŵ�ָ��ģ�鴥����Ӧ��\r\n");
				fmp_error_code = fpm_id_total(&id_total);

				if (fmp_error_code == 0)
				{
					printf("[app_task_fpm383] ��ȡָ��������%04d\r\n", id_total);
					/* ���ָ��*/
					fmp_error_code = fpm_enroll_auto(id_total + 1);
					if (fmp_error_code == 0)
					{
						fpm_ctrl_led(FPM_LED_GREEN);
						printf("[app_task_fpm383] �Զ�ע��ָ�Ƴɹ�\r\n");
						beep.sta = 1;
						beep.duration = 50;
						xReturn = xQueueSend(g_queue_beep, &beep, 100);
						if (xReturn != pdPASS)
							printf("[app_task_fpm383] xQueueSend  beep error code is %d\r\n", xReturn);
						/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
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
			// ɾ��ָ���¼�
			if (EventValue & EVENT_GROUP_FPM_DELETE)
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				fmp_error_code = fpm_empty();
				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					printf("[app_task_fpm383] ���ָ�Ƴɹ�\r\n");
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
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
			// ��ʾָ�������¼�
			if (EventValue & EVENT_GROUP_FPM_SHOW)
			{
				fpm_ctrl_led(FPM_LED_BLUE);
				fmp_error_code = fpm_id_total(&id_total);

				if (fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);
					printf("[app_task_fpm383] ��ȡָ��������%04d\r\n", id_total);
					beep.sta = 1;
					beep.duration = 50;
					xReturn = xQueueSend(g_queue_beep, &beep, 100);
					if (xReturn != pdPASS)
						printf("[app_task_fpm383] xQueueSend beep error code is %d\r\n", xReturn);
					/* [��ѡ]�����ȴ��ź���������ȷ��������ɶ�beep�Ŀ��� */
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
		vTaskResume(app_task_sr04_handle); // �ָ�������ģ��
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
