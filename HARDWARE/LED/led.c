#include "led.h" 

void led_init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	//使能GPIOF时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	//使能GPIOE时钟
	//GPIOF9,F10初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;	//9号和10号引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉
	GPIO_Init(GPIOF, &GPIO_InitStructure);					//初始化GPIO
	GPIO_SetBits(GPIOF,GPIO_Pin_9 | GPIO_Pin_10);			//GPIOF9,F10设置高，灯灭
	//GPIOE13,E14初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;	//13号和14号引脚
	GPIO_Init(GPIOE, &GPIO_InitStructure);					//初始化GPIOE
	GPIO_SetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14);			//GPIOE13,E14设置高，灯灭
}

//呼吸灯初始化
void led_breath_init(void)
{
	GPIO_InitTypeDef  		GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  		TIM_OCInitStructure;	
	//使能GPIOE时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	

	//使能TIM1的硬件时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 ;				//14号引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);					//初始化GPIO
	
	//PE14连接到TIM1
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);	

	//配置TIM1的计数值，决定脉冲的输出频率
	TIM_TimeBaseStructure.TIM_Period = 10000/100-1;	//输出脉冲周期由计数值决定，输出脉冲频率为100Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 16800-1;	//预分频值=16800-1+1=16800分频，第一分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	//时钟分频，也称之为第二次分频，但是F407不支持
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数，从0 -> TIM_Period
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	/* 输出通道3工作在PWM1模式 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	//只要 TIMx_CNT（计数值） < TIMx_CCR1（比较值），通道 1 便为有效状态，否则为无效状态。
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//允许输出脉冲
	TIM_OCInitStructure.TIM_Pulse = 0;	//比较值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//通道的有效状态为低电平
	
	//TIM1的通道4配置
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	
	//默认TIM1不工作
	TIM_Cmd(TIM1, DISABLE);
	
	//默认不使能TIM1的主输出
	TIM_CtrlPWMOutputs(TIM1,DISABLE);	
}

/* 呼吸灯亮度 
参数：
	brightness 0 ~ 100,数值越大，亮度越高
*/
void led_breath_brightness(uint8_t brightness)
{
	TIM_SetCompare4(TIM1,brightness);
}

void led_breath_run(void)
{
	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	led_breath_brightness(0);
}

void led_breath_stop(void)
{
	led_breath_brightness(0);
	TIM_Cmd(TIM1, DISABLE);
	TIM_CtrlPWMOutputs(TIM1,DISABLE);
}






