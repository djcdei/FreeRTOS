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
	// ʹ��tim2��Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// ����tim2�ķ�Ƶֵ������ֵ
	// tim2Ӳ��ʱ��=84MHz/8400=10000Hz�����ǽ���10000�μ���������1��ʱ��ĵ���
	TIM_TimeBaseStructure.TIM_Period = 10000 / 100 - 1; // ����ֵ0 -> 99����10����ʱ��ĵ���
	TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;		// Ԥ��Ƶֵ8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;		// ʱ�ӷ�Ƶ����ǰ��û�еģ�����Ҫ��������
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	// ����tim2���ж�
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

   /* �����ʱ��2��ʱ����±�־λ */
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);	
	/* �رն�ʱ��2 */
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
		dgb_printf_safe("[����ʶ��ģ�� REPLY] �ɹ�\r\n");
	if (reply == 0x01)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ģ��ܾ�������\r\n");
	if (reply == 0x02)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ¼��/ƥ���㷨����ֹ\r\n");
	if (reply == 0x03)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ������Ϣ����\r\n");
	if (reply == 0x04)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] �����ʧ��\r\n");
	if (reply == 0x05)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] δ֪����\r\n");
	if (reply == 0x06)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ��Ч�Ĳ���\r\n");
	if (reply == 0x07)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] �ڴ治��\r\n");
	if (reply == 0x08)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] û����¼����û�\r\n");
	if (reply == 0x09)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ¼�볬������û�����\r\n");
	if (reply == 0x0A)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ������¼��\r\n");
	if (reply == 0x0C)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ������ʧ��\r\n");
	if (reply == 0x0D)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ¼��������ʱ\r\n");
	if (reply == 0x0E)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ����оƬ��Ȩʧ��\r\n");
	if (reply == 0x13)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ���ļ�ʧ��\r\n");
	if (reply == 0x14)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] д�ļ�ʧ��\r\n");
	if (reply == 0x15)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ͨ��Э��δ����\r\n");
	if (reply == 0x17)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] RGBͼ��û��ready\r\n");
	if (reply == 0xFD)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ��Ч��ϢID��keyδд��\r\n");
	if (reply == 0xFE)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] ������\r\n");
	if (reply == 0xFF)
		dgb_printf_safe("[����ʶ��ģ�� REPLY] �������\r\n");
#else

	(void)0;

#endif
}

static void fr_note_info(uint8_t note)
{
#if FR_DEBUG_EN
	if (note == 0x00)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ����\r\n");
	if (note == 0x01)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] δ��⵽����\r\n");
	if (note == 0x02)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ�ϱ��أ�δ��¼��\r\n");
	if (note == 0x03)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ�±��أ�δ��¼��\r\n");
	if (note == 0x04)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ����أ�δ��¼��\r\n");
	if (note == 0x05)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ����̫����ͼƬ�ұ��أ�δ��¼��\r\n");
	if (note == 0x06)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ��������̫Զ��δ��¼��\r\n");
	if (note == 0x07)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ��������̫����δ��¼��\r\n");
	if (note == 0x08)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] üë�ڵ�\r\n");
	if (note == 0x09)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] �۾��ڵ�\r\n");
	if (note == 0x0A)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] �����ڵ�\r\n");
	if (note == 0x0B)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ¼�������������\r\n");
	if (note == 0x0C)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] �ڱ���ģʽ��⵽����״̬\r\n");
	if (note == 0x0D)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] ����״̬\r\n");
	if (note == 0x0E)
		dgb_printf_safe("[����ʶ��ģ�� NOTE] �ڱ���ģʽ������޷��ж�������״̬\r\n");

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

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
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

	/* ���ݰ�����ͷ */
	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;

	/* ���ݰ������� */
	buf_tx[2] = 0x26;

	/* ���ݰ������ݳ��� */
	buf_tx[3] = 0x00;
	buf_tx[4] = 0x28;

	/* ���ݰ�������Ϊ����Ա��ɫ */
	buf_tx[5] = 0x01;

	/* ���ݰ����û��� */
	memcpy(&buf_tx[6], name, strlen(name));

	/* ���ݰ���ע�᷽�򣬵���ע�� */
	buf_tx[38] = 0x01;

	/* ���ݰ���ע������ */
	buf_tx[39] = 0x01;

	/* ���ݰ��������ظ�¼�� */
	buf_tx[40] = 0x01;

	/* ���ݰ�����ʱʱ�� */
	buf_tx[41] = 0x0A;

	/* ���ݰ�������λ 0x00 0x00 0x00 */
	buf_tx[42] = 0x00;
	buf_tx[43] = 0x00;
	buf_tx[44] = 0x00;

	/* ���ݰ���У��λ */
	buf_tx[45] = bcc_check(&buf_tx[2], 43);

	fr_send(buf_tx, 46);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
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

/*�洢��Ӧ���ֵ�������Ϣ*/

int32_t fr_reg_user(const char *name)
{
	uint8_t buf_tx[64] = {0};
	uint8_t buf_rx[USART_PACKET_SIZE] = {0};
	uint32_t timeout = 10000;
	BaseType_t xReturn = pdFALSE;
	/* ���ݰ�����ͷ */
	buf_tx[0] = 0xEF;
	buf_tx[1] = 0xAA;

	/* ���ݰ������� */
	buf_tx[2] = 0x26;

	/* ���ݰ������ݳ��� */
	buf_tx[3] = 0x00;
	buf_tx[4] = 0x28;

	/* ���ݰ�������Ϊ��ͨ�û���ɫ */
	buf_tx[5] = 0x00;

	/* ���ݰ����û��� */
	memcpy(&buf_tx[6], name, strlen(name));

	/* ���ݰ���ע�᷽�򣬵���ע�� */
	buf_tx[38] = 0x01;

	/* ���ݰ���ע������ */
	buf_tx[39] = 0x01;

	/* ���ݰ��������ظ�¼�� */
	buf_tx[40] = 0x01;

	/* ���ݰ�����ʱʱ�� */
	buf_tx[41] = 0x0A;

	/* ���ݰ�������λ 0x00 0x00 0x00 */
	buf_tx[42] = 0x00;
	buf_tx[43] = 0x00;
	buf_tx[44] = 0x00;

	/* ���ݰ���У��λ */
	buf_tx[45] = bcc_check(&buf_tx[2], 43);

	fr_send(buf_tx, 46);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA))
	{
		// ��Ϣ����ΪNOTE
		while (buf_rx[2] == 1)
		{
			xQueueReset(g_queue_frm);

			xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
									buf_rx,		 /* �õ�����Ϣ���� */
									timeout);	 /* �ȴ�ʱ�� */
			if (xReturn != pdPASS)
			{
				dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
				return -1;
			}

			fr_note_info(buf_rx[6]);
		}

		// ��Ϣ����ΪREPLY
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

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
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

	/* ���ݰ���У��λ */
	buf_tx[7] = bcc_check(&buf_tx[2], 5);

	fr_send(buf_tx, 8);

	xQueueReset(g_queue_frm);

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
	if (xReturn != pdPASS)
	{
		dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
		return -1;
	}

	fr_printf_recv(buf_rx, USART_PACKET_SIZE);

	if ((buf_rx[0] == 0xEF) && (buf_rx[1] == 0xAA))
	{

		// ��Ϣ����ΪNOTE
		while (buf_rx[2] == 1)
		{
			xQueueReset(g_queue_frm);

			xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
									buf_rx,		 /* �õ�����Ϣ���� */
									timeout);	 /* �ȴ�ʱ�� */
			if (xReturn != pdPASS)
			{
				dgb_printf_safe("[xQueueReceive] fr_power_down error code is %d\r\n", xReturn);
				return -1;
			}

			fr_note_info(buf_rx[6]);
		}

		// ��Ϣ����ΪREPLY
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

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
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

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
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

	xReturn = xQueueReceive(g_queue_frm, /* ��Ϣ���еľ�� */
							buf_rx,		 /* �õ�����Ϣ���� */
							timeout);	 /* �ȴ�ʱ�� */
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
	/* ��ʱ�� ��ʼ�� */
	tim2_init();

	/* ����1 ��ʼ������ģ���ϵ���������50ms����ܶԸ�ģ�鷢������ */
	usart1_init(baud);

	delay_ms(500);

	return fr_power_down();
}

void TIM2_IRQHandler(void)
{

	uint32_t ulReturn;

	BaseType_t xHigherPriorityTaskWoken;

	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	// ����־λ
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		if (g_usart1_rx_cnt)
		{
			/* �Ӷ���ͷ�������� */
			xQueueSendToFrontFromISR(g_queue_frm, (void *)&g_usart1_rx_buf, &xHigherPriorityTaskWoken);
			memset((void *)g_usart1_rx_buf, 0, sizeof(g_usart1_rx_buf));
			g_usart1_rx_cnt = 0;
		}

		/* �رն�ʱ��2 */
		TIM_Cmd(TIM2, DISABLE);
		// ��ձ�־λ
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		/* ��������Ϣ������������ȼ����ڵ�ǰ���е��������˳��жϺ��������������л���ִ��ǰ��;
	   ����ȴ���һ��ʱ�ӽ��ĲŽ��������л�
	*/
		if (xHigherPriorityTaskWoken)
		{

			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}

		/* �˳��ٽ�� */
		taskEXIT_CRITICAL_FROM_ISR(ulReturn);
	}
}
