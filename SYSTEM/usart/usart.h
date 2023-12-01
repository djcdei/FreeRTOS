#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f4xx_conf.h"
#include "sys.h"
#define USART_PACKET_SIZE 64
extern volatile uint8_t  g_usart1_rx_buf[USART_PACKET_SIZE];
extern volatile uint32_t g_usart1_rx_cnt;
extern volatile uint32_t g_usart1_rx_end;

typedef struct __usart_t
{
	uint8_t rx_buf[USART_PACKET_SIZE];
	uint8_t rx_len;
} usart_t;

// 如果想串口中断接收，请不要注释以下宏定义
extern void usart1_init(u32 baud);
extern void usart2_init(uint32_t baud);
extern void usart3_init(u32 baud);
extern void usart6_init(u32 baud);
extern void usart_send_str(USART_TypeDef* USARTx,char *str);
extern void usart_send_bytes(USART_TypeDef* USARTx,uint8_t *buf,uint32_t len);
extern void usart3_send_str(char *str);
extern void usart3_send_bytes(uint8_t *buf,uint32_t len);


#endif
