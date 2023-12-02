#include "includes.h"

// �������´���,֧��printf����,������Ҫѡ��use MicroLIB
static usart_t g_usart_packet;

volatile uint8_t g_usart1_rx_buf[USART_PACKET_SIZE];
volatile uint32_t g_usart1_rx_cnt = 0;
volatile uint32_t g_usart1_rx_end = 0;

#pragma import(__use_no_semihosting)
// ��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;

// ����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}
void _ttywrch(int ch) {
    // ����Ϊ�գ����߸�����Ҫ�ṩһЩʵ��
}


// �ض���fputc����
int fputc(int ch, FILE *f)
{
	while ((USART6->SR & 0X40) == 0)
		; // ѭ������,ֱ���������
	USART6->DR = (u8)ch;
	return ch;
}

// ��ʼ��IO ����1
// bound:������
void usart1_init(u32 baud)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // ʹ��USART1ʱ��

	// ����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  // GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); // GPIOA10����ΪUSART1

	// USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; // GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// ���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// �ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			// ���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			// ����
	GPIO_Init(GPIOA, &GPIO_InitStructure);					// ��ʼ��PA9��PA10

	// USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = baud;										// ����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// �ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								// ����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// �շ�ģʽ
	USART_Init(USART1, &USART_InitStructure);										// ��ʼ������1

	USART_Cmd(USART1, ENABLE); // ʹ�ܴ���1

	// USART_ClearFlag(USART1, USART_FLAG_TC);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // ��������ж�

	// Usart1 NVIC ���� �����ж���ռ���ȼ����ܸ���configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;													 // ����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY; // ��ռ���ȼ�5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;		 // �����ȼ�5
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;														 // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);																		 // ����ָ���Ĳ�����ʼ��VIC�Ĵ�����
}

void usart2_init(uint32_t baud)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	// ʹ�ܶ˿�AӲ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// ʹ�ܴ���2Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// ����PA2��PA3Ϊ���ù�������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// ��PA2��PA3���ӵ�USART2��Ӳ��
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// ����USART2����ز����������ʡ�����λ��У��λ
	USART_InitStructure.USART_BaudRate = baud;										// ������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 1λֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								// ����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// �����ڷ��ͺͽ�������
	USART_Init(USART2, &USART_InitStructure);

	// ʹ�ܴ��ڽ��յ����ݴ����ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	// ʹ�ܴ���2����
	USART_Cmd(USART2, ENABLE);
}


//����3��ʼ������WIFIͨ��
void usart3_init(uint32_t baud)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//ʹ�ܶ˿�BӲ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	//ʹ�ܴ���3Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	//����PB10��PB11Ϊ���ù�������
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//��PB10��PB11���ӵ�USART3��Ӳ��
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);
	
	
	//����USART1����ز����������ʡ�����λ��У��λ
	USART_InitStructure.USART_BaudRate = baud;//������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1λֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�����ڷ��ͺͽ�������
	USART_Init(USART3, &USART_InitStructure);
	
	
	//ʹ�ܴ��ڽ��յ����ݴ����ж�
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//ʹ�ܴ���3����
	USART_Cmd(USART3,ENABLE);
}


// ����USART6�ĳ�ʼ�����ã���������ͨ�ţ�Ҫע���������ȼ���ʱ���ܸ���configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
void usart6_init(u32 baud)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable GPIO clock */
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	/* Enable USART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	/* Configure USART Tx and Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // �˿ڸ���
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ��ʹ������������
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// ��������PC6 / PC7�Ĺ��ܣ�����ΪUSART6_TX / USART6_RX ������Ӳ��ԭ��ͼ��
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	USART_InitStructure.USART_BaudRate = baud; // ���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// Init USART6
	USART_Init(USART6, &USART_InitStructure);
	/* Enable USART */
	USART_Cmd(USART6, ENABLE);

	// ����Ҫ�жϣ��������ж���ز���
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE); // ����Ϊ:�������ݼĴ��������жϣ�����2��

	USART_ClearITPendingBit(USART6, USART_IT_RXNE); // ��մ��ڽ����жϱ�־λ
	/* Enable the USART6 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;													 // ָ��USART3_IQRͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY; // ��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;		 // ��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;														 // ʹ���ж�����ͨ��
	NVIC_Init(&NVIC_InitStructure);
}

void usart_send_str(USART_TypeDef *USARTx, char *str)
{
	char *p = str;

	while (*p != '\0')
	{
		USART_SendData(USARTx, *p);

		p++;

		// �ȴ����ݷ��ͳɹ�
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
		USART_ClearFlag(USARTx, USART_FLAG_TXE);
	}
}

void usart_send_bytes(USART_TypeDef *USARTx, uint8_t *buf, uint32_t len)
{
	uint8_t *p = buf;

	while (len--)
	{
		USART_SendData(USARTx, *p);

		p++;

		// �ȴ����ݷ��ͳɹ�
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
		USART_ClearFlag(USARTx, USART_FLAG_TXE);
	}
}


void usart3_send_str(char *str)
{
	char *p = str;
	
	while(*p!='\0')
	{
		USART_SendData(USART3,*p);
		
		p++;
		
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
	}
}


void usart3_send_bytes(uint8_t *buf,uint32_t len)
{
	uint8_t *p = buf;
	
	while(len--)
	{
		USART_SendData(USART3,*p);
		
		p++;
		
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
	}
}

// // ����1�жϷ������
// void USART1_IRQHandler(void)
// {
// 	static uint32_t i = 0;

// 	uint8_t data;

// 	uint32_t ulReturn;

// 	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

// 	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
// 	ulReturn = taskENTER_CRITICAL_FROM_ISR();

// 	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
// 	{
// 		// ���մ�������
// 		data = USART_ReceiveData(USART1);

// 		g_usart_packet.rx_buf[i++] = data;

// 		// ��⵽'#'������յ���������ʱ����������
// 		if (data == '#' || i >= (sizeof g_usart_packet.rx_buf))
// 		{
// 			g_usart_packet.rx_len = i;

// 			xQueueSendFromISR(g_queue_usart, (void *)&g_usart_packet, &xHigherPriorityTaskWoken); // ���������ݰ����͵����ڶ���

// 			memset(&g_usart_packet, 0, sizeof g_usart_packet);

// 			i = 0;
// 		}

// 		// ��մ��ڽ����жϱ�־λ
// 		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
// 	}

// 	/* ��������Ϣ������������ȼ����ڵ�ǰ���е��������˳��жϺ��������������л���ִ��ǰ�� */
// 	if (xHigherPriorityTaskWoken)
// 	{
// 		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// 	}

// 	/* �˳��ٽ�� */
// 	taskEXIT_CRITICAL_FROM_ISR(ulReturn);
// }

// ����1����������ʶ��ģ��ͨ��
void USART1_IRQHandler(void)
{
	uint8_t d;

	uint32_t ulReturn;

	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	// ����Ƿ���յ�����
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		d = USART_ReceiveData(USART1);

		if (g_usart1_rx_cnt < sizeof(g_usart1_rx_buf))
		{
			g_usart1_rx_buf[g_usart1_rx_cnt++] = d;
		}

		/* �����ʱ��2��ʱ����±�־λ */
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		/* �رն�ʱ��3 */
		TIM_Cmd(TIM2, DISABLE);

		/* ��յ�ǰ����ֵ */
		TIM_SetCounter(TIM2, 0);

		/* ������ʱ��2 */
		TIM_Cmd(TIM2, ENABLE);

		// ��ձ�־λ��������Ӧ�µ��ж�����
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}

 
void USART6_IRQHandler(void)
{
	static uint32_t i = 0;
	uint8_t data;
	uint32_t ulReturn;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	/* �����ٽ�Σ��ٽ�ο���Ƕ�� */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
	{	
		// ���մ�������
		data = USART_ReceiveData(USART6);
		
		g_usart_packet.rx_buf[i++] = data;
		
		// ��⵽'#'������յ���������ʱ����������
		if (data == '#' || i >= (sizeof g_usart_packet.rx_buf))
		{
			
			g_usart_packet.rx_len = i;

			xQueueSendFromISR(g_queue_usart, (void *)&g_usart_packet, &xHigherPriorityTaskWoken); // ���������ݰ����͵����ڶ���

			memset(&g_usart_packet, 0, sizeof g_usart_packet);

			i = 0;
		}
		// ��մ��ڽ����жϱ�־λ
		USART_ClearITPendingBit(USART6, USART_IT_RXNE);
		
	}

	/* ��������Ϣ������������ȼ����ڵ�ǰ���е��������˳��жϺ��������������л���ִ��ǰ�� */
	if (xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	/* �˳��ٽ�� */
	taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}
