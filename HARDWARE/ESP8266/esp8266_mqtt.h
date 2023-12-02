#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx.h"



//�˴��ǰ����Ʒ������Ĺ���ʵ����½����-------------------------------------ע���޸�Ϊ�Լ����Ʒ����豸��Ϣ��������

// /*smartdevice�Ĳ�������*/
// #define MQTT_BROKERADDRESS 		"k0k7mAaroaa.iot-as-mqtt.cn-shanghai.aliyuncs.com"
// #define MQTT_CLIENTID 			"0001|securemode=3,signmethod=hmacsha1|"
// #define MQTT_USARNAME 			"smartdevice&k0k7mAaroaa"
// #define MQTT_PASSWD 			"D1017DC160B5680387DF7CAA884A5DCB7DF1FF7F"
// #define	MQTT_PUBLISH_TOPIC 		"/sys/k0k7mAaroaa/smartdevice/thing/event/property/post"
// #define MQTT_SUBSCRIBE_TOPIC 	"/sys/k0k7mAaroaa/smartdevice/thing/service/property/set"


/*smartlock�Ĳ�������*/
#define MQTT_BROKERADDRESS 		"k0k7meHPb18.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_CLIENTID 			"0002|securemode=3,signmethod=hmacsha1|"
#define MQTT_USARNAME 			"smartlock&k0k7meHPb18"
#define MQTT_PASSWD 			"BC663D7147073DA47DBC42F873890484BBFF0921"
#define	MQTT_PUBLISH_TOPIC 		"/sys/k0k7meHPb18/smartlock/thing/event/property/post"
#define MQTT_SUBSCRIBE_TOPIC 	"/sys/k0k7meHPb18/smartlock/thing/service/property/set"


//�˴��ǰ����Ʒ���������ҵʵ����½����-------------------------------------ע���޸�Ϊ�Լ����Ʒ����豸��Ϣ��������
//#define MQTT_BROKERADDRESS 		"iot-060a065f.mqtt.iothub.aliyuncs.com"
//#define MQTT_CLIENTID 			"0001|securemode=3,signmethod=hmacsha1|"
//#define MQTT_USARNAME 			"smartdevice&g850YXdgU5r"
//#define MQTT_PASSWD 			"A8F93BD31F6085B1AB2AE3CC311E38971B15885D"
//#define	MQTT_PUBLISH_TOPIC 		"/sys/g850YXdgU5r/smartdevice/thing/event/property/post"
//#define MQTT_SUBSCRIBE_TOPIC 	"/sys/g850YXdgU5r/smartdevice/thing/service/property/set"


#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	

//MQTT���ӷ�����
extern int32_t mqtt_connect(char *client_id,char *user_name,char *password);

//MQTT��Ϣ����
extern int32_t mqtt_subscribe_topic(char *topic,uint8_t qos,uint8_t whether);

//MQTT��Ϣ����
extern uint32_t mqtt_publish_data(char *topic, char *message, uint8_t qos);

//MQTT����������
extern void mqtt_send_heart(void);

extern int32_t esp8266_mqtt_init(void);

extern void mqtt_report_devices_status(void);

#endif
