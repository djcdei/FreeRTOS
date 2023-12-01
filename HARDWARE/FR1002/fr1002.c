#include "includes.h"

static NVIC_InitTypeDef NVIC_InitStructure;
static TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

static uint8_t bcc_check(uint8_t *buf, uint32_t len)
{
	uint8_t s = 0;
	uint8_t i = 0;
	uint8_t *p = buf;

	for (i = 0; i < len; i++)
		s = s ^ p[i];
	return s;
}

static void tim2_init(void)
{
	// 使能tim2的硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// 配置tim2的分频值、计数值
	// tim2硬件时钟=84MHz/8400=10000Hz，就是进行10000次计数，就是1秒时间的到达
	TIM_TimeBaseStructure.TIM_Period = 10000 / 100 - 1; // 计数值0 -> 99就是10毫秒时间的到达
	TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;		// 预分频值8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;		// 时钟分频，当前是没有的，不需要进行配置
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	// 配置tim2的中断
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

   /* 清除定时器2的时间更新标志位 */
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);	
	/* 关闭定时器2 */
	TIM_Cmd(TIM2, DISABLE);
}

static void fr_printf_recv(uint8_t *buf, uint32_t len)
{

#if FR_DEBUG_EN

	uint8_t *p = buf;
	uint32_t i;

	dgb_printf_safe("fr recv buf:");

	for (i = 0; i < len; i++)
	{
		printf("%02X ", p[i]);
	}

	dgb_printf_safe("\r\n");

#else

	(void)0;

#endif
}

static void fr_reply_info(uint8_t reply)
{
#if FR_DEBUG_EN
	if (reply == 0x00)
		dgb_printf_safe("[人脸识别模块 REPLY] 成功\r\n");
	if (reply == 0x01)
		dgb_printf_safe("[人脸识别模块 REPLY] 模组拒绝该命令\r\n");
	if (reply == 0x02)
		dgb_printf_safe("[人脸识别模块 REPLY] 录入/匹配算法已终止\r\n");
	if (reply == 0x03)
		dgb_printf_safe("[人脸识别模块 REPLY] 发送消息错误\r\n");
	if (reply == 0x04)
		dgb_printf_safe("[人脸识别模块 REPLY] 相机打开失败\r\n");
	if (reply == 0x05)
		dgb_printf_safe("[人脸识别模块 REPLY] 未知错误\r\n");
	if (reply == 0x06)
		dgb_printf_safe("[人脸识别模块 REPLY] 无效的参数\r\n");
	if (reply == 0x07)
		dgb_printf_safe("[人脸识别模块 REPLY] 内存不足\r\n");
	if (reply == 0x08)
		dgb_printf_safe("[人脸识别模块 REPLY] 没有已录入的用户\r\n");
	if (reply == 0x09)
		dgb_printf_safe("[人脸识别模块 REPLY] 录入超过最大用户数量\r\n");
	if (reply == 0x0A)
		dgb_printf_safe("[人脸识别模块 REPLY] 人脸已录入\r\n");
	if (reply == 0x0C)
		dgb_printf_safe("[人脸识别模块 REPLY] 活体检测失败\r\n");
	if (reply == 0x0D)
		dgb_printf_safe("[人脸识别模块 REPLY] 录入或解锁超时\r\n");
	if (reply == 0x0E)
		dgb_printf_safe("[人脸识别模块 REPLY] 加密芯片授权失败\r\n");
	if (reply == 0x13)
		dgb_printf_safe("[人脸识别模块 REPLY] 读文件失败\r\n");
	if (reply == 0x14)
		dgb_printf_safe("[人脸识别模块 REPLY] 写文件失败\r\n");
	if (reply == 0x15)
		dgb_printf_safe("[人脸识别模块 REPLY] 通信协议未加密\r\n");
	if (reply == 0x17)
		dgb_printf_safe("[人脸识别模块 REPLY] RGB图像没有ready\r\n");
	if (reply == 0xFD)
		dgb_printf_safe("[人脸识别模块 REPLY] 无效信息ID，key未写入\r\n");
	if (reply == 0xFE)
		dgb_printf_safe("[人脸识别模块 REPLY] 检测错误\r\n");
	if (reply == 0xFF)
		dgb_printf_safe("[人脸识别模块 REPLY] 编码错误\r\n");
#else

	(void)0;

#endif
}

static void fr_note_info(uint8_t note)
{
#if FR_DEBUG_EN
	if (note == 0x00)
		dgb_printf_safe("[人脸识别模块 NOTE] 人脸\r\n");
	if (note == 0x01)
		dgb_printf_safe("[人脸识别模块 NOTE] 未检测到人脸\r\n");
	if (note == 0x02)
		dgb_printf_safe("[人脸识别模块 NOTE] 人脸太靠近图片上边沿，未能录入\r\n");
	if (note == 0x03)
		dgb_printf_safe("[人脸识别模块 NOTE] 人脸太靠近图片下边沿，未能录入\r\n");
	if (note == 0x04)
		dgb_printf_safe("[人脸识别模块 NOTE] 人脸太靠近图片左边沿，未能录入\r\n");
	if (note == 0x05)
		dgb_printf_safe("[人脸识别模块 NOTE] 人脸太靠近图片右边沿，未能录入\r\n");
	if (note == 0x06)
		dgb_printf_safe("[人脸识别模块 NOTE] 人脸距离太远，未能录入\r\n");
	if (note == 0x07)
		dgb_printf_safe("[人脸识别模块 NOTE] 人脸距离太近，未能录入\r\n");
	if (note == 0x08)
		dgb_printf_safe("[人脸识别模块 NOTE] 眉毛遮挡\r\n");
	if (note == 0x09)
		dgb_printf_safe("[人脸识别模块 NOTE] 眼睛遮挡\r\n");
	if (note == 0x0A)
		dgb_printf_safe("[人脸识别模块 NOTE] 脸部遮挡\r\n");
	if (note == 0x0B)
		dgb_printf_safe("[人脸识别模块 NOTE] 录入人脸方向错误\r\n");
	if (note == 0x0C)
		dgb_printf_safe("[人脸识别模块 NOTE] 在闭眼模式检测到睁眼状态\r\n");
	if (note == 0x0D)
		dgb_printf_safe("[人脸识别模块 NOTE] 闭眼状态\r\n");
	if (note == 0x0E)
		dgb_printf_safe("[人脸识别模块 NOTE] 在闭眼模式检测中无法判定睁闭眼状态\r\n");

#else

	(void)0;

#endif
}

void fr_send(uint8_t *buf, uint32_t len)
{
	uint8_t *p = buf;

	// g_usart1_rx_end = 0;
	// g_usart1_rx_cnt = 0;

	while (len--)
	{
		USART_SendData(USART1, *p++);

		while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE))
			;
		USART_ClearFlag(USART1, USART_FLAG_TXE);
	}
}

int32_t fr_entry_standby(void)
{
	uint8_t buf_tx[6] = {0};
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;
	BaseType_t xReturn = pdFALSE;
	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;
	buf_tx[2] = 0x23;
	buf_tx[3] = 0x00;
	buf_tx[4] = 0x00;
	buf_tx[5] = 0x23;

	fr_send(buf_tx, 6);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA) && (buf_rx[5] == 0x23))
	{
		if (buf_rx[6])
		{
			if (buf_rx[2] == 0)
			{
				fr_reply_info(buf_rx[6]);
			}

			if (buf_rx[2] == 1)
			{
				fr_note_info(buf_rx[6]);
			}

			return buf_rx[6];
		}
		else
			return 0;
	}

	return -1;
}

int32_t fr_reg_admin(const char *name)
{
	uint8_t buf_tx[64] = {0};
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;
	BaseType_t xReturn = pdFALSE;

	/* 数据包：包头 */
	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;

	/* 数据包：命令 */
	buf_tx[2] = 0x26;

	/* 数据包：数据长度 */
	buf_tx[3] = 0x00;
	buf_tx[4] = 0x28;

	/* 数据包：设置为管理员角色 */
	buf_tx[5] = 0x01;

	/* 数据包：用户名 */
	memcpy(&buf_tx[6], name, strlen(name));

	/* 数据包：注册方向，单向注册 */
	buf_tx[38] = 0x01;

	/* 数据包：注册类型 */
	buf_tx[39] = 0x01;

	/* 数据包：允许重复录入 */
	buf_tx[40] = 0x01;

	/* 数据包：超时时间 */
	buf_tx[41] = 0x0A;

	/* 数据包：保留位 0x00 0x00 0x00 */
	buf_tx[42] = 0x00;
	buf_tx[43] = 0x00;
	buf_tx[44] = 0x00;

	/* 数据包：校验位 */
	buf_tx[45] = bcc_check(&buf_tx[2], 43);

	fr_send(buf_tx, 46);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA) && (buf_rx[5] == 0x26))
	{
		if (buf_rx[6])
		{
			if (buf_rx[2] == 0)
			{
				fr_reply_info(buf_rx[6]);
			}

			if (buf_rx[2] == 1)
			{
				fr_note_info(buf_rx[6]);
			}

			return buf_rx[6];
		}
		else
			return 0;
	}

	return -1;
}

/*存储对应名字的人脸信息*/

int32_t fr_reg_user(const char *name)
{
	uint8_t buf_tx[64] = {0};
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;
	BaseType_t xReturn = pdFALSE;
	/* 数据包：包头 */
	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;

	/* 数据包：命令 */
	buf_tx[2] = 0x26;

	/* 数据包：数据长度 */
	buf_tx[3] = 0x00;
	buf_tx[4] = 0x28;

	/* 数据包：设置为普通用户角色 */
	buf_tx[5] = 0x00;

	/* 数据包：用户名 */
	memcpy(&buf_tx[6], name, strlen(name));

	/* 数据包：注册方向，单向注册 */
	buf_tx[38] = 0x01;

	/* 数据包：注册类型 */
	buf_tx[39] = 0x01;

	/* 数据包：允许重复录入 */
	buf_tx[40] = 0x01;

	/* 数据包：超时时间 */
	buf_tx[41] = 0x0A;

	/* 数据包：保留位 0x00 0x00 0x00 */
	buf_tx[42] = 0x00;
	buf_tx[43] = 0x00;
	buf_tx[44] = 0x00;

	/* 数据包：校验位 */
	buf_tx[45] = bcc_check(&buf_tx[2], 43);

	fr_send(buf_tx, 46);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA))
	{
		// 消息队列为NOTE
		while (buf_rx[2] == 1)
		{
			xQueueReset(g_queue_frm);

			xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
									buf_rx,		 /* 得到的消息内容 */
									timeout);	 /* 等待时间 */
			if (xReturn != pdPASS)
			{
				dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
				return -1;
			}

			fr_note_info(buf_rx[6]);
		}

		// 消息类型为REPLY
		if (buf_rx[2] == 0)
		{
			fr_reply_info(buf_rx[6]);
			if (buf_rx[6])
				return buf_rx[6];

			return 0;
		}
	}

	return -1;
}

int32_t fr_del_user_all(void)
{
	uint8_t buf_tx[8] = {0};
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;

	BaseType_t xReturn = pdFALSE;
	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;

	buf_tx[2] = 0x21;

	buf_tx[3] = 0x00;
	buf_tx[4] = 0x00;

	buf_tx[5] = 0x21;

	fr_send(buf_tx, 6);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA) && (buf_rx[5] == 0x21))
	{

		if (buf_rx[6])
		{
			if (buf_rx[2] == 0)
			{
				fr_reply_info(buf_rx[6]);
			}

			if (buf_rx[2] == 1)
			{
				fr_note_info(buf_rx[6]);
			}

			return buf_rx[6];
		}
		else
			return 0;
	}
	return -1;
}

int32_t fr_match(uint8_t *buf)
{
	uint8_t buf_tx[8] = {0};
	static uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;

	BaseType_t xReturn = pdFALSE;

	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;

	buf_tx[2] = 0x12;

	buf_tx[3] = 0x00;
	buf_tx[4] = 0x02;

	buf_tx[5] = 0x0A;
	buf_tx[6] = 0x0A;

	/* 数据包：校验位 */
	buf_tx[7] = bcc_check(&buf_tx[2], 5);

	fr_send(buf_tx, 8);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA))
	{

		// 消息队列为NOTE
		while (buf_rx[2] == 1)
		{
			xQueueReset(g_queue_frm);

			xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
									buf_rx,		 /* 得到的消息内容 */
									timeout);	 /* 等待时间 */
			if (xReturn != pdPASS)
			{
				dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
				return -1;
			}

			fr_note_info(buf_rx[6]);
		}

		// 消息类型为REPLY
		if (buf_rx[2] == 0)
		{
			fr_reply_info(buf_rx[6]);
			if (buf_rx[6])
				return buf_rx[6];

			memcpy(buf, (void *)&buf_rx[7], 36);
			return 0;
		}
	}

	return -1;
}

int32_t fr_state_get(void)
{
	uint8_t buf_tx[6] = {0};
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;

	BaseType_t xReturn = pdFALSE;

	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;
	buf_tx[2] = 0x11;
	buf_tx[3] = 0x00;
	buf_tx[4] = 0x00;
	buf_tx[5] = 0x11;

	fr_send(buf_tx, 6);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	return 0;
}

int32_t fr_get_user_total(void)
{
	int32_t user_total = 0;
	uint32_t timeout = 4000;
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	BaseType_t xReturn = pdFALSE;

	uint8_t buf_tx[6] = {0};

	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;

	buf_tx[2] = 0x24;

	buf_tx[3] = 0x00;
	buf_tx[4] = 0x00;

	buf_tx[5] = 0x24;

	fr_send(buf_tx, 6);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA))
	{
		if ((buf_rx[2] == 0) && (buf_rx[5] == 0x24) && (buf_rx[6] == 0))
		{
			user_total = buf_rx[7];

			return user_total;
		}
	}

	return -1;
}

int32_t fr_power_down(void)
{
	uint8_t buf_tx[6] = {0};
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;

	BaseType_t xReturn = pdFALSE;

	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;
	buf_tx[2] = 0xED;
	buf_tx[3] = 0x00;
	buf_tx[4] = 0x00;
	buf_tx[5] = 0xED;

	fr_send(buf_tx, 6);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* 消息队列的句柄 */
							buf_rx,		 /* 得到的消息内容 */
							timeout);	 /* 等待时间 */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA))
	{
		if ((buf_rx[2] == 0) && (buf_rx[5] == 0xED) && (buf_rx[6] == 0x00))
		{

			return 0;
		}
	}

	return -1;
}

int32_t fr_init(uint32_t baud)
{
	/* 定时器 初始化 */
	tim2_init();

	/* 串口1 初始化，当模块上电后，最起码等50ms后才能对该模块发送数据 */
	usart1_init(baud);

	delay_ms(500);

	return fr_power_down();
}

void TIM2_IRQHandler(void)
{

	uint32_t ulReturn;

	BaseType_t xHigherPriorityTaskWoken;

	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	// 检测标志位
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		if (g_usart1_rx_cnt)
		{
			/* 从队列头插入数据 */
			xQueueSendToFrontFromISR(g_queue_frm, (void *)&g_usart1_rx_buf, &xHigherPriorityTaskWoken);
			memset((void *)g_usart1_rx_buf, 0, sizeof(g_usart1_rx_buf));
			g_usart1_rx_cnt = 0;
		}

		/* 关闭定时器2 */
		TIM_Cmd(TIM2, DISABLE);
		// 清空标志位
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		/* 若接收消息队列任务的优先级高于当前运行的任务，则退出中断后立即进行任务切换，执行前者;
	   否则等待下一个时钟节拍才进行任务切换
	*/
		if (xHigherPriorityTaskWoken)
		{

			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}

		/* 退出临界段 */
		taskEXIT_CRITICAL_FROM_ISR(ulReturn);
	}
}
