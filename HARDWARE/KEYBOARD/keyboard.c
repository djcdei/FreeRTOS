/* ����:��Ƕ.������ */
#include "includes.h"

// ��
#define R1 PGout(12)
#define R2 PDout(5)
#define R3 PDout(15)
#define R4 PDout(1)
// ��
#define C1 PEin(7)
#define C2 PDin(14)
#define C3 PDin(4)
#define C4 PFin(12)

void kbd_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // ʹ�ܶ˿�D��Ӳ��ʱ�ӣ����ǶԶ˿�D����
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    // ʹ�ܶ˿�E��Ӳ��ʱ�ӣ����ǶԶ˿�E����
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    // ʹ�ܶ˿�F��Ӳ��ʱ�ӣ����ǶԶ˿�F����
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // ʹ�ܶ˿�G��Ӳ��ʱ�ӣ����ǶԶ˿�G����
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;    // ����Ϊ����ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    // ��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed; // ����Ϊ���٣���Ҫ����ʱ���Ϊ�͵�ƽ
    // ��ʼ��GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; // ����Ϊ�˿�F��0������
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_14; // ����Ϊ�˿�F��0������
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; // ����Ϊ�˿�F��0������
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // ��
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   // ����Ϊ���ģʽ
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    // �������������裬���ٹ���
    GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed; // ����Ϊ���٣���Ҫ����ʱ���Ϊ�͵�ƽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // ����pp(push pull),��©(open drain)

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; // ����Ϊ�˿�F��0������
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_15 | GPIO_Pin_1; // ����Ϊ�˿�F��0������
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

    /* ʹ��״̬��˼��õ�������״̬ */
    switch (key_sta)
    {
    case 0: // ��ȡ���µİ���
    {
        key_cur = _kbd_read();

        if (key_cur != 'N')
        {
            key_old = key_cur;
            key_sta = 1;
        }
    }
    break;

    case 1: // ȷ�ϰ��µİ���
    {

        key_cur = _kbd_read();

        if ((key_cur != 'N') && (key_cur == key_old))
        {
            key_sta = 2;
        }
    }
    break;

    case 2: // ��ȡ�ͷŵİ���
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
