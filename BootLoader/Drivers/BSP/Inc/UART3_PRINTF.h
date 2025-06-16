#ifndef __UART3_PRINTF_H
#define __UART3_PRINTF_H


#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

void UART3_IDLE_CallBack(void);
int _write(int fd, char *ptr, int len);
void UART3_Init(void);

#endif
