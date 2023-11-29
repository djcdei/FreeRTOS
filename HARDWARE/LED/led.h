#ifndef __LED_H
#define __LED_H
#include "sys.h"


//LED端口定义
#define LED0 PFout(9)	// DS0
#define LED1 PFout(10)	// DS1	
#define LED2 PEout(13)
#define LED3 PEout(14)

/*LED状态(led_sta)宏定义*/
#define LED1_ON 	0x11
#define LED1_OFF 	0x10
#define LED2_ON 	0x21
#define LED2_OFF 	0x20
#define LED3_ON 	0x41
#define LED3_OFF 	0x40
#define LED4_ON 	0x81
#define LED4_OFF 	0x80

extern void led_init(void);//初始化
extern void led_breath_init(void);	
extern void led_breath_brightness(uint8_t brightness);	
extern void led_breath_run(void); 	
extern void led_breath_stop(void);			    
#endif
