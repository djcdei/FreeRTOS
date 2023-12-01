#include "sr04.h" 
#include <stdio.h>
static GPIO_InitTypeDef GPIO_InitStructure;
/*������ģ���ʼ��*/
void sr04_init(void)
{
	// �򿪶˿�B��Ӳ��ʱ�ӣ����ǶԶ˿�B����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	// �򿪶˿�E��Ӳ��ʱ�ӣ����ǶԵ�ǰӲ������
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	// ����PB6�˿�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 // ��6������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	 // �������ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // �����������ǿ�������������ŵ������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;	 // ����ģʽ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // û��ʹ���ڲ���������
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// ����ʱ��ͼ��PB6��ǰΪ�͵�ƽ
	PBout(6) = 0;

	// ����PE6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 // ��6������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	 // ���빦��ģʽ���ȿ������������ƣ�Ҳ��������������ƣ�
	GPIO_InitStructure.GPIO_Speed = GPIO_Low_Speed;	 // ����ģʽ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // û��ʹ���ڲ���������
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}
/*����ʱ��ͼ��װ��������ȡ������Ϣ�ĺ���*/
int32_t sr04_get_distance(void)
{
	int32_t t=0;
	PBout(6)=1;
	delay_us(20);
	PBout(6)=0;
	
	//�ȴ������źű�Ϊ�ߵ�ƽ
	while(PEin(6)==0)
	{
		t++;
		delay_us(1);
		
		//�����ʱ���ͷ���һ��������
		if(t>=1000000)
			return -1;
	}
	
	t=0;
	//�����ߵ�ƽ������ʱ��
	while(PEin(6))
	{
	
		//��ʱ9us,����3mm�Ĵ������
		delay_us(9);
		
		t++;
		
		//�����ʱ���ͷ���һ��������
		if(t>=100000)
			return -1;		
	
	}
	//printf("distance :%d\n",3*(t/2));
	//��ǰ�Ĵ������
	return 3*(t/2);
}







