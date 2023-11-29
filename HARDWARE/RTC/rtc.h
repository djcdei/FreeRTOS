#ifndef __RTC_H__
#define __RTC_H__

extern void rtc_init(void);
extern void rtc_resume_init(void);
extern uint8_t to_hex(uint32_t num);
extern void rtc_alarm_init(uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t AlarmDateWeekDay);
#endif

