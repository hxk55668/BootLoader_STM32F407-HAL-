#include "UART3_PRINTF.h"
#include "U1_BootLoader.h"

#define ESP01_BUFFER_SIZE		1024
uint8_t g_UART3_Rx_Buffer[ESP01_BUFFER_SIZE];

extern uint8_t g_uart2_rx_buffer[ESP01_BUFFER_SIZE];

int _write(int fd, char *ptr, int len) {
    HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, 1000);
    return len;
}


void UART3_Init(void)
{
	  __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
	  HAL_UART_Receive_DMA(&huart3, g_UART3_Rx_Buffer, strlen(g_UART3_Rx_Buffer));
}

void UART3_IDLE_CallBack(void)
{
	  uint8_t idle = __HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE);
	      uint8_t UART3_Tx_Buffer[ESP01_BUFFER_SIZE];

	      if(RESET != idle) {
	          __HAL_UART_CLEAR_IDLEFLAG(&huart3);
	          HAL_UART_DMAStop(&huart3);

	          strcpy(UART3_Tx_Buffer, g_UART3_Rx_Buffer);
	          //»ØÏÔ·¢ËÍ×Ö·û
	          HAL_UART_Transmit(&huart3, UART3_Tx_Buffer, strlen(UART3_Tx_Buffer), 100);
//						U1_Printf((char *)UART3_Tx_Buffer);

	          memset(g_UART3_Rx_Buffer, '\0', 256);

	          HAL_UART_Receive_DMA(&huart3, g_UART3_Rx_Buffer, ESP01_BUFFER_SIZE);
	          HAL_UART_Receive_DMA(&huart2,g_uart2_rx_buffer,ESP01_BUFFER_SIZE);
	      }
}
