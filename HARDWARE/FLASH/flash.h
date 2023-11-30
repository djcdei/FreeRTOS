#ifndef __FLASH_H_
#define __FLASH_H_
#include "sys.h"
#include "stm32f4xx_flash.h"
#define FLASH_SAVE_ADDR ADDR_FLASH_SECTOR_4	  // 扇区4别名
#define ADDR_FLASH_SECTOR_4 ((u32)0x08010000) // 扇区4起始地址

typedef struct __flash_t
{
#define MODE_OPEN_LOCK_KEYBOARD 0x01 // 按键解锁数据
#define MODE_OPEN_LOCK_BLUE 0x02	 // 蓝牙解锁数据
#define MODE_OPEN_LOCK_DHT 0x04		 // 温湿度数据
#define MODE_OPEN_LOCK_RFID 0x08	 // RFID解锁数据
#define MODE_OPEN_LOCK_SFM 0x10		 // 指纹解锁
#define MODE_OPEN_LOCK_FRM 0X20		 // 人脸解锁

	uint32_t offset;	 // 位置
	uint8_t mode;		 // 模式(选择上面的宏定义模式)
	uint8_t databuf[64]; // 记录一些数据
	uint8_t date[6];	 // 记录日期
} flash_t;

extern void flash_init(void);
extern void flash_write(flash_t *flash_write_buf);
extern void flash_read(uint32_t num);
extern uint32_t flash_read_offset(uint32_t *start_addr);
#endif
