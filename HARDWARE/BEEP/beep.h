#ifndef __BEEP_H__
#define __BEEP_H__
#include "includes.h"
extern void beep_init(void);//��ʼ��
/*beep��Ϣ�ṹ*/
typedef struct __beep_t
{
	uint32_t sta;	   // 1-���� 0-ֹͣ
	uint32_t duration; // ����ʱ�䣬��λ����
} beep_t;

#define BEEP(x)		PFout(8)=(x)


#endif
