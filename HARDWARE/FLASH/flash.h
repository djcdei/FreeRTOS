#ifndef __FLASH_H_
#define __FLASH_H_
#include "sys.h"
#include "stm32f4xx_flash.h"
#define FLASH_SAVE_ADDR ADDR_FLASH_SECTOR_4	  // ����4����
#define ADDR_FLASH_SECTOR_4 ((u32)0x08010000) // ����4��ʼ��ַ

typedef struct __flash_t
{
#define MODE_OPEN_LOCK_KEYBOARD 0x01 // ������������
#define MODE_OPEN_LOCK_BLUE 0x02	 // ������������
#define MODE_OPEN_LOCK_DHT 0x04		 // ��ʪ������
#define MODE_OPEN_LOCK_RFID 0x08	 // RFID��������
#define MODE_OPEN_LOCK_SFM 0x10		 // ָ�ƽ���
#define MODE_OPEN_LOCK_FRM 0X20		 // ��������

	uint32_t offset;	 // λ��
	uint8_t mode;		 // ģʽ(ѡ������ĺ궨��ģʽ)
	uint8_t databuf[64]; // ��¼һЩ����
	uint8_t date[6];	 // ��¼����
} flash_t;

extern void flash_init(void);
extern void flash_write(flash_t *flash_write_buf);
extern void flash_read(uint32_t num);
extern uint32_t flash_read_offset(uint32_t *start_addr);
#endif
