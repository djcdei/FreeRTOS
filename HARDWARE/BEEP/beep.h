#ifndef __BEEP_H__
#define __BEEP_H__
#include "includes.h"
extern void beep_init(void);//初始化
/*beep消息结构*/
typedef struct __beep_t
{
	uint32_t sta;	   // 1-工作 0-停止
	uint32_t duration; // 持续时间，单位毫秒
} beep_t;

#define BEEP(x)		PFout(8)=(x)


#endif
