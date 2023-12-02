#ifndef __INCLUDES_H__
#define __INCLUDES_H__

/* ��׼C��*/
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
/* ������� */
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

/*RFID��Ƶ��״̬�궨��*/
#define STA_CARD_REQUEST 0 // RFID����
#define STA_CARD_FOUND 1
#define STA_CARD_VALID_FOUND 2
#define STA_CARD_INVALID_FOUND 3
#define STA_CARD_VALID_ADD 4

#define MAX_CARDS 100 // ��Ƶ��ע���������

/* ������ص��¼���־�궨�� */
#define EVENT_GROUP_KEY1_DOWN 0x01
#define EVENT_GROUP_KEY2_DOWN 0x02
#define EVENT_GROUP_KEY3_DOWN 0x04
#define EVENT_GROUP_KEY4_DOWN 0x08
#define EVENT_GROUP_KEYALL_DOWN 0x0F // �¼���

/*RTCʵʱʱ������¼���־���жϴ���*/
#define EVENT_GROUP_RTC_WAKEUP 0x10 // ʵʱʱ�ӻ����¼�
#define EVENT_GROUP_RTC_ALARM 0x20	// �����¼�

#define EVENT_GROUP_DHT 0x40
#define EVENT_GROUP_SR04 0x80

#define EVENT_GROUP_OLED_ON 0x100
#define EVENT_GROUP_OLED_OFF 0x200

#define EVENT_GROUP_SYSTEM_RESET 0x1000

/*ָ��ģ����ص��¼���־*/
#define EVENT_GROUP_FPM_NONE 0x2000	  // ָ��ģ��δ����
#define EVENT_GROUP_FPM_ADD 0X4000	  // ���ָ��
#define EVENT_GROUP_FPM_DELETE 0X8000 // ɾ��ָ��
#define EVENT_GROUP_FPM_AUTH 0X10000  // ��ָ֤��
#define EVENT_GROUP_FPM_SHOW 0X20000  // ��ʾ��ǰָ�Ƹ���

/*3D����ʶ��ģ����ص��¼���־*/
#define EVENT_GROUP_FACE_ADD 0X40000	// �������
#define EVENT_GROUP_FACE_DELETE 0X80000 // ɾ������
#define EVENT_GROUP_FACE_AUTH 0X100000	// ��֤����
#define EVENT_GROUP_FACE_SHOW 0X200000	// ��ʾ��ǰ��������
#define EVENT_GROUP_FACE_OK 0x400000	// ����ʶ����ɱ�־
#define EVENT_GROUP_FACE_AGAIN 0x800000	//����ʶ��ʧ�ܣ��ٴ�ʶ��

/*���г��Ⱥ궨��*/
#define QUEUE_USART_LEN 4 /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define QUEUE_LED_LEN 4	  /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define QUEUE_BEEP_LEN 4  /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define QUEUE_OLED_LEN 16 /* ���еĳ��ȣ����ɰ������ٸ���Ϣ */
#define QUEUE_FLASH_LEN 16
#define QUEUE_KEYBOARD_LEN 4 /*������̶��г���*/
#define QUEUE_FSM_LEN 16 
#define QUEUE_MOTOR_LEN 1 
#define QUEUE_ESP8266_LEN 3

/*rtc��־λ*/
#define FLAG_RTC_GET_NONE 0
#define FLAG_RTC_GET_DATE 1 // ��ȡ��ǰ����
#define FLAG_RTC_GET_TIME 2 // ��ȡ��ǰʱ��
/*��ȡ���Ҷȱ�־λ*/
#define FLAG_DHT_GET_NONE 0
#define FLAG_DHT_GET_START 1
/*��������־λ*/
#define FLAG_PASS_MAN_NONE 0   // �޲���
#define FLAG_PASS_MAN_AUTH 1   // ������֤
#define FLAG_PASS_MAN_MODIFY 2 // �����޸�
/*������־λ*/
#define FLAG_UNLOCK_OK 1 // �豸�ѽ���
#define FLAG_UNLOCK_NO 0 // �豸������

#define PASS1_ADDR_IN_EEPROM 0 // e2promд���ʼ��ַ
#define PASS_LEN 6			   // ���볤��
#define XOR_KEY 0X88		   // ������Կ

/* ���� */
/*�����ź������*/
extern SemaphoreHandle_t g_mutex_printf;
/* ��ȫ��ӡ���� */
extern void dgb_printf_safe(const char *format, ...);

/* ���� */
extern float g_temp;
extern float g_humi;
extern volatile uint32_t g_unlock_what;

/*�¼���־��*/
extern EventGroupHandle_t g_event_group;

/*���ڶ��о��*/
extern QueueHandle_t g_queue_usart;
/* �������о�� */
extern QueueHandle_t 	g_queue_frm;

/* wifi���о�� */
extern QueueHandle_t 	g_queue_esp8266;

/* ���Ӳ����Ϣ���ͽṹ��� */
extern volatile uint32_t g_ISR_dbg;//�ж��жϷ������Ƿ񱻴����ĵ��Ա�־λ

/*oled��Ϣ�ṹ*/
typedef struct __oled_t
{

#define OLED_CTRL_DISPLAY_ON 0x01  // oled����
#define OLED_CTRL_DISPLAY_OFF 0x02 // oledϢ��
#define OLED_CTRL_INIT 0x03
#define OLED_CTRL_CLEAR 0x04
#define OLED_CTRL_SHOW_STRING 0x05	// oled��ʾ�ַ���
#define OLED_CTRL_SHOW_CHINESE 0x06 // oled��ʾ������
#define OLED_CTRL_SHOW_PICTURE 0x07 // oled��ʾͼƬ

	uint8_t ctrl;
	uint8_t x;
	uint8_t y;

	uint8_t *str;	   // �����ʾ�ַ����׵�ַ
	uint8_t font_size; // �����С
	uint8_t chinese;

	const uint8_t *pic; // ���ͼƬ�����׵�ַ
	uint8_t pic_width;	// ͼƬ��
	uint8_t pic_height; // ͼƬ��
} oled_t;

/*��������ṹ��*/
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
