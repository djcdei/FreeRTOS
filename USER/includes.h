#ifndef __INCLUDES_H__
#define __INCLUDES_H__

/* 标准C库*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
/* 外设相关 */
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "dht11.h"
#include "sr04.h"
#include "flash.h"
#include "key.h"
#include "keyboard.h"
#include "esp8266.h"
#include "esp8266_mqtt.h"
#include "at24c02.h"
#include "fpm383.h"
#include "fr1002.h"
#include "motor.h"
// #include "mpu6050.h"
// #include "inv_mpu.h"
// #include "inv_mpu_dmp_motion_driver.h"
// #include "dmpKey.h"
// #include "dmpmap.h"
#include "rtc.h"
// #include "iwdg.h"
#include "beep.h"
#include "oled.h"
#include "oledfont.h"
#include "bmp.h"

//#include "cJSON.h"

/*RFID射频卡状态宏定义*/
#define STA_CARD_REQUEST 0 // RFID请求
#define STA_CARD_FOUND 1
#define STA_CARD_VALID_FOUND 2
#define STA_CARD_INVALID_FOUND 3
#define STA_CARD_VALID_ADD 4

#define MAX_CARDS 100 // 射频卡注册最大数量

/* 按键相关的事件标志宏定义 */
#define EVENT_GROUP_KEY1_DOWN 0x01
#define EVENT_GROUP_KEY2_DOWN 0x02
#define EVENT_GROUP_KEY3_DOWN 0x04
#define EVENT_GROUP_KEY4_DOWN 0x08
#define EVENT_GROUP_KEYALL_DOWN 0x0F // 事件或

/*RTC实时时钟相关事件标志，中断触发*/
#define EVENT_GROUP_RTC_WAKEUP 0x10 // 实时时钟唤醒事件
#define EVENT_GROUP_RTC_ALARM 0x20	// 闹钟事件

#define EVENT_GROUP_DHT 0x40
#define EVENT_GROUP_SR04 0x80

#define EVENT_GROUP_OLED_ON 0x100
#define EVENT_GROUP_OLED_OFF 0x200

#define EVENT_GROUP_SYSTEM_RESET 0x1000

/*指纹模块相关的事件标志*/
#define EVENT_GROUP_FPM_NONE 0x2000	  // 指纹模块未启动
#define EVENT_GROUP_FPM_ADD 0X4000	  // 添加指纹
#define EVENT_GROUP_FPM_DELETE 0X8000 // 删除指纹
#define EVENT_GROUP_FPM_AUTH 0X10000  // 验证指纹
#define EVENT_GROUP_FPM_SHOW 0X20000  // 显示当前指纹个数

/*3D人脸识别模块相关的事件标志*/
#define EVENT_GROUP_FACE_ADD 0X40000	// 添加人脸
#define EVENT_GROUP_FACE_DELETE 0X80000 // 删除人脸
#define EVENT_GROUP_FACE_AUTH 0X100000	// 验证人脸
#define EVENT_GROUP_FACE_SHOW 0X200000	// 显示当前人脸个数
#define EVENT_GROUP_FACE_OK 0x400000	// 人脸识别完成标志
#define EVENT_GROUP_FACE_AGAIN 0x800000	//人脸识别失败，再次识别

/*队列长度宏定义*/
#define QUEUE_USART_LEN 4 /* 队列的长度，最大可包含多少个消息 */
#define QUEUE_LED_LEN 4	  /* 队列的长度，最大可包含多少个消息 */
#define QUEUE_BEEP_LEN 4  /* 队列的长度，最大可包含多少个消息 */
#define QUEUE_OLED_LEN 16 /* 队列的长度，最大可包含多少个消息 */
#define QUEUE_FLASH_LEN 16
#define QUEUE_KEYBOARD_LEN 4 /*矩阵键盘队列长度*/
#define QUEUE_FSM_LEN 16 
#define QUEUE_MOTOR_LEN 1 
#define QUEUE_ESP8266_LEN 3

/*rtc标志位*/
#define FLAG_RTC_GET_NONE 0
#define FLAG_RTC_GET_DATE 1 // 获取当前日期
#define FLAG_RTC_GET_TIME 2 // 获取当前时间
/*获取温室度标志位*/
#define FLAG_DHT_GET_NONE 0
#define FLAG_DHT_GET_START 1
/*密码管理标志位*/
#define FLAG_PASS_MAN_NONE 0   // 无操作
#define FLAG_PASS_MAN_AUTH 1   // 密码验证
#define FLAG_PASS_MAN_MODIFY 2 // 密码修改
/*解锁标志位*/
#define FLAG_UNLOCK_OK 1 // 设备已解锁
#define FLAG_UNLOCK_NO 0 // 设备已上锁

#define PASS1_ADDR_IN_EEPROM 0 // e2prom写入初始地址
#define PASS_LEN 6			   // 密码长度
#define XOR_KEY 0X88		   // 解密密钥

/* 变量 */
/*互斥信号量句柄*/
extern SemaphoreHandle_t g_mutex_printf;
/* 安全打印函数 */
extern void dgb_printf_safe(const char *format, ...);

/* 变量 */
extern float g_temp;
extern float g_humi;
extern volatile uint32_t g_unlock_what;

/*事件标志组*/
extern EventGroupHandle_t g_event_group;

/*串口队列句柄*/
extern QueueHandle_t g_queue_usart;
/* 人脸队列句柄 */
extern QueueHandle_t 	g_queue_frm;

/* wifi队列句柄 */
extern QueueHandle_t 	g_queue_esp8266;

/* 相关硬件消息类型结构设计 */
extern volatile uint32_t g_ISR_dbg;//判断中断服务函数是否被触发的调试标志位

/*oled消息结构*/
typedef struct __oled_t
{

#define OLED_CTRL_DISPLAY_ON 0x01  // oled亮屏
#define OLED_CTRL_DISPLAY_OFF 0x02 // oled息屏
#define OLED_CTRL_INIT 0x03
#define OLED_CTRL_CLEAR 0x04
#define OLED_CTRL_SHOW_STRING 0x05	// oled显示字符串
#define OLED_CTRL_SHOW_CHINESE 0x06 // oled显示中文字
#define OLED_CTRL_SHOW_PICTURE 0x07 // oled显示图片

	uint8_t ctrl;
	uint8_t x;
	uint8_t y;

	uint8_t *str;	   // 存放显示字符串首地址
	uint8_t font_size; // 字体大小
	uint8_t chinese;

	const uint8_t *pic; // 存放图片数据首地址
	uint8_t pic_width;	// 图片宽
	uint8_t pic_height; // 图片高
} oled_t;

/*构建任务结构体*/
typedef struct __task_t
{
	TaskFunction_t pxTaskCode;
	const char *const pcName;
	const configSTACK_DEPTH_TYPE usStackDepth;
	void *const pvParameters;
	UBaseType_t uxPriority;
	TaskHandle_t *const pxCreatedTask;
} task_t;

#endif
