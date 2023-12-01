#include "includes.h"

void motor_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	
	/* 使能对应的GPIOA GPIOB GPIOC GPIOE 时钟*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		   // 第4根引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 设置为输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽模式，增加驱动电流
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不需要上拉电阻
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		   // 第7根引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 设置为输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽模式，增加驱动电流
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不需要上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;		   // 第15根引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 设置为输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽模式，增加驱动电流
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不需要上拉电阻
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		   // 第9根引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // 设置为输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // 推挽模式，增加驱动电流
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 不需要上拉电阻
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	MOTOR_IN1 = MOTOR_IN2 = MOTOR_IN3 = MOTOR_IN4 = 1;
}


// 双四拍驱动方式正转表360度 AD-DC-CB-BA
void motor_corotation_double_pos(void)
{
	uint32_t i = 0, j = 0;

	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 8; j++)
		{
			MOTOR_IN4 = 0;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 0;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 0;
			delay_ms(2);
		}
	}

	MOTOR_IN1 = MOTOR_IN2 = MOTOR_IN3 = MOTOR_IN4 = 1;
}

// 双四拍驱动方式反转表360度 AB-BC-CD-DA
void motor_corotation_double_rev(void)
{
	uint32_t i = 0, j = 0;

	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 8; j++)
		{
			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 0;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 0;
			delay_ms(2);
		}
	}

	MOTOR_IN1 = MOTOR_IN2 = MOTOR_IN3 = MOTOR_IN4 = 1;
}


// 八拍驱动方式正转表360度 AD-D-DC-C-CB-B-BA-A
void motor_corotation_eghit_pos(void)
{
	uint32_t i = 0, j = 0;

	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 8; j++)
		{
			MOTOR_IN4 = 0;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 0;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 0;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 0;
			delay_ms(2);
		}
	}

	MOTOR_IN1 = MOTOR_IN2 = MOTOR_IN3 = MOTOR_IN4 = 1;
}
// 八拍驱动方式反转表 A-AB-B-BC-C-CD-D-DA
void motor_corotation_eghit_rev(void)
{
	uint32_t i = 0, j = 0;

	for (i = 0; i < 64; i++)
	{
		for (j = 0; j < 8; j++)
		{
			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 0;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 0;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 0;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 1;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 0;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 1;
			delay_ms(2);

			MOTOR_IN4 = 0;
			MOTOR_IN3 = 1;
			MOTOR_IN2 = 1;
			MOTOR_IN1 = 0;
			delay_ms(2);
		}
	}

	MOTOR_IN1 = MOTOR_IN2 = MOTOR_IN3 = MOTOR_IN4 = 1;
}
