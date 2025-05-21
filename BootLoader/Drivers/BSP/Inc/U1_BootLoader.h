#ifndef __U1_BOOTLOADER_H
#define __U1_BOOTLOADER_H

#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#define BootLoader_Handle   huart1
#define U1TX_Bufffer_Size   2048
#define U1RX_Bufffer_Size   2048
#define U1RX_Buffer_Max     256
#define URXData_Buffer_Size 10
//定义记录接收和结束数据的指针结构体
typedef struct{
    uint8_t *start;
    uint8_t *end;
}UCB_RxBufferPtr;

typedef struct 
{   
    //记录累加值和存放SE指针的数组
    uint16_t U1RX_Counter;
    UCB_RxBufferPtr URxDataBuffer[URXData_Buffer_Size];
    //记录接收数据和处理数据速度
    UCB_RxBufferPtr *URxDataIn;
    UCB_RxBufferPtr *URxDataOut;
    UCB_RxBufferPtr *URxDataEnd;
}UCB_ControlBlock;
//函数声明
void UCBRx_PtrInit(void);
void U1BootLoader_Init(void);
void U1RXBootLoader_IDLE_CallBack(UART_HandleTypeDef *huart);
void U1_Printf(char *format, ...);
//外部声明
extern UCB_ControlBlock UCB_CB;
extern uint8_t U1_RX_Buffer[U1RX_Bufffer_Size];

#endif
