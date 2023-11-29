/* 作者:粤嵌.温子祺 */
#include "includes.h"

// 行
#define R1 PGout(12)
#define R2 PDout(5)
#define R3 PDout(15)
#define R4 PDout(1)
// 列
#define C1 PEin(7)
#define C2 PDin(14)
#define C3 PDin(4)
#define C4 PFin(12)

void kbd_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能端口D的硬件时钟，就是对端口D供电
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    // 使能端口E的硬件时钟，就是对端口E供电
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    // 使能端口F的硬件时钟，就是对端口F供电
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // 使能端口G的硬件时钟，就是对端口G供电
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;    // 设置为输入模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    // 上拉电阻
    GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed; // 设置为低速，需要更长时间变为低电平
    // 初始化GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; // 设置为端口F的0号引脚
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_14; // 设置为端口F的0号引脚
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; // 设置为端口F的0号引脚
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 行
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   // 设置为输出模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    // 无上拉下拉电阻，减少功耗
    GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed; // 设置为低速，需要更长时间变为低电平
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // 推挽pp(push pull),开漏(open drain)

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; // 设置为端口F的0号引脚
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_15 | GPIO_Pin_1; // 设置为端口F的0号引脚
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

static char _kbd_read(void)
{
    // R1 R2 R3 R4
    R1 = 0;
    R2 = 1;
    R3 = 1;
    R4 = 1;
    delay_ms(2);

    // C1 C2 C3 C4
    if (C4 == 0)
        return '1';
    else if (C3 == 0)
        return '2';
    else if (C2 == 0)
        return '3';
    else if (C1 == 0)
        return 'A';

    R1 = 1;
    R2 = 0;
    R3 = 1;
    R4 = 1;
    delay_ms(2);

    // PD1 PD15 PD5 PG12
    if (C4 == 0)
        return '4';
    else if (C3 == 0)
        return '5';
    else if (C2 == 0)
        return '6';
    else if (C1 == 0)
        return 'B';

    R1 = 1;
    R2 = 1;
    R3 = 0;
    R4 = 1;
    delay_ms(2);
    // PD1 PD15 PD5 PG12
    if (C4 == 0)
        return '7';
    else if (C3 == 0)
        return '8';
    else if (C2 == 0)
        return '9';
    else if (C1 == 0)
        return 'C';

    R1 = 1;
    R2 = 1;
    R3 = 1;
    R4 = 0;
    delay_ms(2);
    // PD1 PD15 PD5 PG12
    if (C4 == 0)
        return '*';
    else if (C3 == 0)
        return '0';
    else if (C2 == 0)
        return '#';
    else if (C1 == 0)
        return 'D';

    return 'N';
}

char kbd_read(void)
{
    static char key_sta = 0;
    static char key_old = 'N';
    char key_val = 'N';
    char key_cur = 'N';

    /* 使用状态机思想得到按键的状态 */
    switch (key_sta)
    {
    case 0: // 获取按下的按键
    {
        key_cur = _kbd_read();

        if (key_cur != 'N')
        {
            key_old = key_cur;
            key_sta = 1;
        }
    }
    break;

    case 1: // 确认按下的按键
    {

        key_cur = _kbd_read();

        if ((key_cur != 'N') && (key_cur == key_old))
        {
            key_sta = 2;
        }
    }
    break;

    case 2: // 获取释放的按键
    {

        key_cur = _kbd_read();

        if (key_cur == 'N')
        {
            key_val = key_old;
            key_sta = 0;
            key_old = 'N';
        }
    }
    break;

    default:
        break;
    }

    return key_val;
}
