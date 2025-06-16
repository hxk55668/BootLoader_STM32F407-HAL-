#ifndef __ESP01_UART_H
#define __ESP01_UART_H

#include "main.h"
#include "driver_esp.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "U1_BootLoader.h"
#include "BootLoader.h"

#define DHT11_clientId		"k164odR0WDC.DHT11|securemode=2\\,signmethod=hmacsha256\\,timestamp=1742812489509|"
#define DHT11_username		"DHT11&k164odR0WDC"
#define DHT11_passwd		"3e6c6e3c18c436bf556eeea594cb39d145e94d7bf0d6795a8f5581f8beb91bc6"
#define	DHT11_mqttHostUrl	"iot-06z00j2bwfnre1l.mqtt.iothub.aliyuncs.com"

#define DHT11_SET			"/sys/k164odR0WDC/DHT11/thing/service/property/set"
#define DHT11_POST			"/sys/k164odR0WDC/DHT11/thing/event/property/post"


#define MQ2_clientID		"k164odR0WDC.MQ-2|securemode=2\\,signmethod=hmacsha256\\,timestamp=1742901286012|"
#define MQ2_username		"MQ-2&k164odR0WDC"
#define MQ2_passwd			"f25f0783a833537d22182ee34ab66a93c6d32de341e09de4241e2f01af2f4452"
#define MQ2_mqttHostUrl		"iot-06z00j2bwfnre1l.mqtt.iothub.aliyuncs.com"

#define MQ2_SET				"/sys/k164odR0WDC/MQ-2/thing/service/property/set"
#define MQ2_POST			"/sys/k164odR0WDC/MQ-2/thing/service/property/post"


void ESP8266_Init(void);
void ESP01_IDLE_CallBack(void);
void AliYun_Init(void);
void AliYun_Topic(void);
void ESP8266_GetWeather(uint8_t day);
uint16_t esp8266_copy_rxdata(char *data, uint16_t max_len);
void ESP8266_RESET(void);

#endif
