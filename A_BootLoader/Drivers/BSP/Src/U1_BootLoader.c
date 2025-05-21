#include "U1_BootLoader.h"

//缓冲区数组 最大2048
uint8_t U1_RX_Buffer[U1RX_Bufffer_Size];
uint8_t U1_TX_Buffer[U1TX_Bufffer_Size];
//数据控制结构体
UCB_ControlBlock UCB_CB;
//记录单次接收数据的长度
volatile uint8_t Rx_Len = 0;


/**
 * @brief   初始化指针
 * @param   None
 * @retval  None
 */
void UCBRx_PtrInit(void)
{
    UCB_CB.URxDataIn    = &UCB_CB.URxDataBuffer[0];
    UCB_CB.URxDataOut   = &UCB_CB.URxDataBuffer[0];
    UCB_CB.URxDataEnd   = &UCB_CB.URxDataBuffer[URXData_Buffer_Size - 1];
    UCB_CB.URxDataIn->start = U1_RX_Buffer;
    UCB_CB.U1RX_Counter = 0;
}
/**
 * @brief   初始化串口1 开启DMA接收和空闲中断并初始化UCB指针
 * @param   None
 * @retval  None
 */
void U1BootLoader_Init(void)
{
    UCBRx_PtrInit();
    __HAL_UART_ENABLE_IT(&BootLoader_Handle, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&BootLoader_Handle, UCB_CB.URxDataIn->start, U1RX_Buffer_Max);
}

/**
 * @brief   U1串口空闲中断回调函数 
 * @param   None
 * @retval  None
 */
void U1RXBootLoader_IDLE_CallBack(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1 && __HAL_UART_GET_FLAG(&BootLoader_Handle, UART_IT_IDLE)) {
      __HAL_UART_CLEAR_IDLEFLAG(huart);
    }
    //计算接收数据长度
    __disable_irq();
    Rx_Len = U1RX_Buffer_Max - __HAL_DMA_GET_COUNTER(huart -> hdmarx);
    __enable_irq();
    UCB_CB.U1RX_Counter += Rx_Len;                          //记录缓冲区数组已用长度
    UCB_CB.URxDataIn->end = &U1_RX_Buffer[UCB_CB.U1RX_Counter - 1]; //记录本次接收数据的长度
    UCB_CB.URxDataIn++;                                             //本次In指针记录完成后向后移
    if (UCB_CB.URxDataIn == UCB_CB.URxDataEnd){                     //判断是否SE数组是否满了
        UCB_CB.URxDataIn    = &UCB_CB.URxDataBuffer[0];
    }
    //检查数据缓冲区是否满
    if(U1RX_Bufffer_Size - UCB_CB.U1RX_Counter >= 256){             //判断缓冲区数组是否满了 若没满则更新start位置
        UCB_CB.URxDataIn->start = &U1_RX_Buffer[UCB_CB.U1RX_Counter];
    }else{
        UCB_CB.URxDataIn->start = U1_RX_Buffer;
        UCB_CB.U1RX_Counter = 0;
    }
		
    HAL_UART_AbortReceive(&BootLoader_Handle);               // 终止当前DMA接收
    HAL_UART_Receive_DMA(&BootLoader_Handle, UCB_CB.URxDataIn->start, U1RX_Buffer_Max); // 重启DMA
    Rx_Len = 0;
  }
/**
 * @brief   U1 Printf重定向
 * @param   format 格式字符串
 * @retval  None
 */
void U1_Printf(char *format, ...)
{
    va_list listdata;
    va_start(listdata, format);
    int len = vsnprintf((char*)U1_TX_Buffer, sizeof(U1_TX_Buffer), format, listdata);
    va_end(listdata);

    if (len > 0) {
        // 一次性发送有效数据（非整个数组）
        HAL_UART_Transmit(&BootLoader_Handle, U1_TX_Buffer, len, 100);
    }
    // 可选：等待最后一个字节发送完成（确保硬件完成）
    while (__HAL_UART_GET_FLAG(&BootLoader_Handle, UART_FLAG_TC) == RESET);
}
