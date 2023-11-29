#include "includes.h"
// 加入以下代码,支持printf函数,而不需要选择use MicroLIB
static usart_t g_usart_packet;

#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;

// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
// 重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART3->SR & 0X40) == 0)
		; // 循环发送,直到发送完毕
	USART3->DR = (u8)ch;
	return ch;
}

// 初始化IO 串口1
// bound:波特率
void usart1_init(u32 baud)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // 使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // 使能USART1时钟

	// 串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  // GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); // GPIOA10复用为USART1

	// USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; // GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// 复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			// 推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			// 上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);					// 初始化PA9，PA10

	// USART1 初始化设置
	USART_InitStructure.USART_BaudRate = baud;										// 波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(USART1, &USART_InitStructure);										// 初始化串口1

	USART_Cmd(USART1, ENABLE); // 使能串口1

	// USART_ClearFlag(USART1, USART_FLAG_TC);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 开启相关中断

	// Usart1 NVIC 配置 串口中断抢占优先级不能高于configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;													 // 串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY; // 抢占优先级5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;		 // 子优先级5
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;														 // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);																		 // 根据指定的参数初始化VIC寄存器、
}

void usart2_init(uint32_t baud)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	// 使能端口A硬件时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// 使能串口2硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// 配置PA2、PA3为复用功能引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// 将PA2、PA3连接到USART2的硬件
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// 配置USART2的相关参数：波特率、数据位、校验位
	USART_InitStructure.USART_BaudRate = baud;										// 波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 8位数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 允许串口发送和接收数据
	USART_Init(USART2, &USART_InitStructure);

	// 使能串口接收到数据触发中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	// 使能串口2工作
	USART_Cmd(USART2, ENABLE);
}

// 串口USART3的初始化配置，用于蓝牙通信，要注意配置优先级的时候不能高于configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
void usart3_init(u32 baud)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	/* Enable USART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* Configure USART Tx and Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // 端口复用
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不使能上下拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 复用引脚PB10 / PB11的功能，配置为USART3_TX / USART3_RX （查阅硬件原理图）
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

	USART_InitStructure.USART_BaudRate = baud; // 设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// Init USART3
	USART_Init(USART3, &USART_InitStructure);
	/* Enable USART */
	USART_Cmd(USART3, ENABLE);

	// 若需要中断，则配置中断相关参数
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 设置为:接收数据寄存器不空中断（参数2）

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;													 // 指定USART3_IQR通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;		 // 响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;														 // 使能中断请求通道
	NVIC_Init(&NVIC_InitStructure);
}

// 串口1中断服务程序
void USART1_IRQHandler(void)
{
	static uint32_t i = 0;

	uint8_t data;

	uint32_t ulReturn;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		// 接收串口数据
		data = USART_ReceiveData(USART1);

		g_usart_packet.rx_buf[i++] = data;

		// 检测到'#'符或接收的数据满的时候则发送数据
		if (data == '#' || i >= (sizeof g_usart_packet.rx_buf))
		{
			g_usart_packet.rx_len = i;

			xQueueSendFromISR(g_queue_usart, (void *)&g_usart_packet, &xHigherPriorityTaskWoken); // 将串口数据包发送到串口队列

			memset(&g_usart_packet, 0, sizeof g_usart_packet);

			i = 0;
		}

		// 清空串口接收中断标志位
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

	/* 若接收消息队列任务的优先级高于当前运行的任务，则退出中断后立即进行任务切换，执行前者 */
	if (xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}

// 串口3中断服务程序,
void USART3_IRQHandler(void)
{
	static uint32_t i = 0;

	uint8_t data;

	uint32_t ulReturn;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	/* 进入临界段，临界段可以嵌套 */
	ulReturn = taskENTER_CRITICAL_FROM_ISR();

	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		// 接收串口数据
		data = USART_ReceiveData(USART3);

		g_usart_packet.rx_buf[i++] = data;

		// 检测到'#'符或接收的数据满的时候则发送数据
		if (data == '#' || i >= (sizeof g_usart_packet.rx_buf))
		{
			g_usart_packet.rx_len = i;

			xQueueSendFromISR(g_queue_usart, (void *)&g_usart_packet, &xHigherPriorityTaskWoken); // 将串口数据包发送到串口队列

			memset(&g_usart_packet, 0, sizeof g_usart_packet);

			i = 0;
		}

		// 清空串口接收中断标志位
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}

	/* 若接收消息队列任务的优先级高于当前运行的任务，则退出中断后立即进行任务切换，执行前者 */
	if (xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	/* 退出临界段 */
	taskEXIT_CRITICAL_FROM_ISR(ulReturn);
}
