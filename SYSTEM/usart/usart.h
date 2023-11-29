#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f4xx_conf.h"
#include "sys.h"

typedef struct __usart_t
{
	uint8_t rx_buf[64];
	uint8_t rx_len;
} usart_t;

// ����봮���жϽ��գ��벻Ҫע�����º궨��
extern void usart1_init(u32 baud);
extern void usart2_init(uint32_t baud);
extern void usart3_init(u32 baud);

#endif
