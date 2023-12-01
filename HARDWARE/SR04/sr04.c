#include "sr04.h" 
#include <stdio.h>
static GPIO_InitTypeDef GPIO_InitStructure;
/*超声波模块初始化*/
void sr04_init(void)
{
	// 打开端口B的硬件时钟，就是对端口B供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	// 打开端口E的硬件时钟，就是对当前硬件供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	// 配置PB6端口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 // 第6号引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	 // 输出功能模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // 推挽输出，增强驱动能力，引脚的输出电流更大
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;	 // 低速模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 没有使用内部上拉电阻
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 根据时序图，PB6当前为低电平
	PBout(6) = 0;

	// 配置PE6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 // 第6号引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	 // 输入功能模式（既可以软件代码控制，也可以其他外设控制）
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;	 // 低速模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 没有使用内部上拉电阻
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}
/*根据时序图封装超声波获取距离信息的函数*/
int32_t sr04_get_distance(void)
{
	int32_t t=0;
	PBout(6)=1;
	delay_us(20);
	PBout(6)=0;
	
	//等待回响信号变为高电平
	while(PEin(6)==0)
	{
		t++;
		delay_us(1);
		
		//如果超时，就返回一个错误码
		if(t>=1000000)
			return -1;
	}
	
	t=0;
	//测量高电平持续的时间
	while(PEin(6))
	{
	
		//延时9us,就是3mm的传输距离
		delay_us(9);
		
		t++;
		
		//如果超时，就返回一个错误码
		if(t>=100000)
			return -1;		
	
	}
	//printf("distance :%d\n",3*(t/2));
	//当前的传输距离
	return 3*(t/2);
}







