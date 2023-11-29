/*EEPROM*/

#include "includes.h"


/*模拟I2C初始化*/
void at24c02_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	// 打开端口B的时钟（看硬件原理图）
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	// 配置端口B的8、9号引脚，配置为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; // 8 9号引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		   // 输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		   // 推挽 Push Pull；开漏 Open Drain
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	   // 不使能上下拉电阻
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;		   // 低速，功耗低，但是引脚响应时间更长
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 配置引脚电平（看时序图）
	AT24C02_SCL_W = 1; // PB8
	AT24C02_SDA_W = 1; // PB9
}

// PB9引脚模式设置函数
void at24c02_sda_pin_mode(GPIOMode_TypeDef pin_mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		 // 9号引脚
	GPIO_InitStructure.GPIO_Mode = pin_mode;		 // 输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	 // 推挽 Push Pull；开漏 Open Drain
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不使能上下拉电阻
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;	 // 低速，功耗低，但是引脚响应时间更长
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// i2c启动函数(信号)
void at24c02_i2c_start(void)
{
	// 设置PB9为输出模式
	at24c02_sda_pin_mode(GPIO_Mode_OUT);
	// 软件控制起始信号的发生，起始信号：时钟为高电平的时候，数据线SDA由高到低的变化，定义为起始信号（看文档）
	AT24C02_SCL_W = 1; // 时钟高电平
	AT24C02_SDA_W = 1; // 软件代码设置数据线从高到低发生跳变
	delay_us(5);	   // 延时5us（半个时钟周期），I2C时钟为100KHz也就是周期10us
	AT24C02_SDA_W = 0;
	delay_us(5); // 延时5us
	// 时钟拉低
	AT24C02_SCL_W = 0;
	delay_us(5);
}

// i2c停止函数(信号)
void at24c02_i2c_stop(void)
{
	// 设置PB9为输出模式
	at24c02_sda_pin_mode(GPIO_Mode_OUT);
	AT24C02_SCL_W = 1; // 时钟高电平
	AT24C02_SDA_W = 0; // 软件代码设置数据线从低到高发生跳变
	delay_us(5);
	AT24C02_SDA_W = 1;
	delay_us(5);
}

// i2c发送数据函数
void at24c02_i2c_send_byte(uint8_t byte)
{
	int32_t i;
	at24c02_sda_pin_mode(GPIO_Mode_OUT); // 保证SDA引脚为输出模式
	AT24C02_SCL_W = 0;
	AT24C02_SDA_W = 0;
	delay_us(5);

	for (i = 7; i >= 0; i--)
	{
		if (byte & (1 << i))
			AT24C02_SDA_W = 1;
		else
			AT24C02_SDA_W = 0;

		delay_us(5);
		// 当前SDA引脚电平是不变的，然后从机可以可靠访问

		AT24C02_SCL_W = 1;
		delay_us(5);

		// 当前SDA引脚电平可能会发生变更，然后从机访问是不可靠
		AT24C02_SCL_W = 0;
		delay_us(5);
	}
}
// i2c接收数据函数
uint8_t at24c02_i2c_recv_byte(void)
{
	uint8_t data = 0;
	int32_t i;
	at24c02_sda_pin_mode(GPIO_Mode_IN); // SDA引脚切换为输入模式用于接收数据
	// 高位先收
	for (i = 7; i >= 0; i--)
	{
		AT24C02_SCL_W = 1;
		delay_us(5);
		if (AT24C02_SDA_R) // 判断PBin(9)电平高低
		{
			data |= 1 << i;
		}
		AT24C02_SCL_W = 0; // 保持总线占用
		delay_us(5);
	}
	return data;
}

// i2c等待读取从机应答信号
uint8_t at24c02_i2c_wait_ack(void)
{
	uint8_t ack = 0;
	at24c02_sda_pin_mode(GPIO_Mode_IN); // 将PB(9)转化为输入模式

	AT24C02_SCL_W = 1;
	delay_us(5);

	// 检测到SDA引脚为低电平，就是从机有应答
	if (AT24C02_SDA_R == 0)
		ack = 0;
	else
		ack = 1;

	AT24C02_SCL_W = 0;
	delay_us(5);
	return ack;
}

// i2c主机发送ack应答信号
void at24c02_i2c_ack(uint8_t ack)
{
	// 保证SDA引脚为输出模式
	at24c02_sda_pin_mode(GPIO_Mode_OUT);
	AT24C02_SCL_W = 0;
	AT24C02_SDA_W = 0;
	delay_us(5);

	if (ack)
		AT24C02_SCL_W = 1;
	else
		AT24C02_SDA_W = 0;

	delay_us(5);

	AT24C02_SCL_W = 1;
	delay_us(5);

	AT24C02_SCL_W = 0;
	delay_us(5);
}

// i2c写入数据
int32_t at24c02_write(uint8_t addr, uint8_t *buf, uint32_t len)
{
	uint8_t ack;
	uint8_t *p = buf;

	// 发送起始信号
	at24c02_i2c_start();

	// 进行设备寻址，7位设备地址加上以为写访问位（读：1，写0）
	at24c02_i2c_send_byte(0xA0);
	// 等待应答信号ack
	ack = at24c02_i2c_wait_ack();

	if (ack)
	{
		printf("device address error\r\n");
		return -1;
	}

	// 发送要写入数据的eeprom起始地址
	at24c02_i2c_send_byte(addr);
	// 等待ack应答
	ack = at24c02_i2c_wait_ack();
	if (ack)
	{
		printf("word address error\r\n");
		return -2;
	}

	// 连续发送数据
	while (len--)
	{
		at24c02_i2c_send_byte(*p++);
		// 每发送完一字节数据就要等到一次ack应答信号
		ack = at24c02_i2c_wait_ack();
		if (ack)
		{
			printf("write data error\r\n");
			return -3;
		}
	}
	// 数据发送完成，发送停止信号
	at24c02_i2c_stop();
	printf("write data success\r\n");
	return 0;
}

// i2c读取数据
int32_t at24c02_read(uint8_t addr, uint8_t *buf, uint32_t len)
{
	uint8_t ack;
	uint8_t *p = buf;

	// 发送起始信号
	at24c02_i2c_start();

	// 进行设备寻址，7位设备地址加上以为写访问位（读：1，写0）
	at24c02_i2c_send_byte(0xA0);
	// 等待ACK应答
	ack = at24c02_i2c_wait_ack();
	if (ack)
	{
		printf("device address error\r\n");
		return -1;
	}
	// 发送要访问e2prom数据起始地址
	at24c02_i2c_send_byte(addr);
	// 等待ACK应答
	ack = at24c02_i2c_wait_ack();
	if (ack)
	{
		printf("device address error\r\n");
		return -1;
	}

	// 再次发送起始信号，并从写访问切换到读访问

	at24c02_i2c_start();

	// 进行设备寻址，7位设备地址加上以为写访问位（读：1，写0）
	at24c02_i2c_send_byte(0xA1);

	// 等待ACK应答
	ack = at24c02_i2c_wait_ack();
	if (ack)
	{
		printf("device address error\r\n");
		return -1;
	}

	// 连续读取数据
	len = len - 1;
	while (len--)
	{
		// 读取每一节数据
		*p = at24c02_i2c_recv_byte();
		// 发送应答信号
		at24c02_i2c_ack(0);
		p++;
	}
	// 读取最后一个字节
	*p = at24c02_i2c_recv_byte();
	// 发送应答信号
	at24c02_i2c_ack(0);

	// 发送停止信号
	at24c02_i2c_stop();

	return 0;
}