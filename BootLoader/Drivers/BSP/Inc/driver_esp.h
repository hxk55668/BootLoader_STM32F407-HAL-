#ifndef __DRIVER_ESP_H
#define __DRIVER_ESP_H

#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

//ESP8266模式选择
typedef enum
{
    STA,
    AP,
    STA_AP
}ENUM_Net_ModeTypeDef;

_Bool ESP8266_Enable_Passthrough(void) ;
void esp01_uart_send_data(uint8_t *data,uint16_t data_len);
_Bool ESP8266_TEST(void);//测试命令 发送“AT”
_Bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode); //设置模式 STA AP STA_AP
_Bool ESP8266_Set_MultiplesMode(uint8_t mode);//选择多链路 1多 0单
_Bool ESP8266_JoinAP(void);//加入WIFI
_Bool ESP8266_Connect_TCPServer(uint8_t id, const char* ip, uint16_t port);//建立TCP服务器
_Bool ESP8266_Send_Cmd(char *cmd, char *res);//发送命名
_Bool ESP8266_Create_TCPServer(uint16_t port);//建立TCP服务器
_Bool ESP8266_Send_Str(char* str, uint8_t uid, uint16_t strlen);
_Bool ESP8266_Set_SNTP_Config(void);
_Bool ESP8266_Send_Str(char* str, uint8_t uid, uint16_t length);
#endif
