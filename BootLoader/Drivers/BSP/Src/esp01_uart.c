#include "esp01_uart.h"

#define ESP01_BUFFER_SIZE		1024
#define IP						"api.seniverse.com"
#define PORT					80

volatile uint16_t ESP01_RX_LEN = 0;					//用于计算接收到数据的长度
extern uint8_t g_UART3_Rx_Buffer[ESP01_BUFFER_SIZE];
extern uint8_t g_esp8266_rx_buffer[ESP01_BUFFER_SIZE];   // ESP8266响应数据接收缓冲区
#define ESP01_UART_HANDLE	huart2
uint8_t g_uart2_rx_buffer[ESP01_BUFFER_SIZE];


/**
  * @brief   ESP8266初始化
  * @param   None
  * @retval  None
 **/
void ESP8266_Init(void)
{
//	__HAL_UART_ENABLE_IT(&ESP01_UART_HANDLE, UART_IT_IDLE);
//	HAL_UART_Receive_DMA(&ESP01_UART_HANDLE, g_uart2_rx_buffer, ESP01_BUFFER_SIZE);
	//ESP测试AT指令
	while (!ESP8266_TEST());
	HAL_Delay(500);
	//配置为STA模式
	while (!ESP8266_Net_Mode_Choose(STA));
	HAL_Delay(500);
	//设置单链路模式
	while (!ESP8266_Set_MultiplesMode(0));
	HAL_Delay(500);
	//连接WIFI
	while (! ESP8266_JoinAP());
	HAL_Delay(2000);
	//连接TCP服务器
	while (!ESP8266_Connect_TCPServer(0, IP, PORT));
	HAL_Delay(500);
	while(!ESP8266_Send_Cmd("AT+CIPMODE=1\r\n", "OK")) HAL_Delay(500);
	while(!ESP8266_Send_Cmd("AT+CIPSEND\r\n", ">")) HAL_Delay(500);
}
/**
  * @brief   获取天气
  * @param   None
  * @retval  None
 **/
void ESP8266_GetWeather(uint8_t day) {
    // 构造符合HTTP标准的请求
    char request[256];
    snprintf(request, sizeof(request),"GET https://api.seniverse.com/v3/weather/daily.json?key=S3sPxT5_ExrRyH0If&location=chengdu&language=zh-Hans&unit=c&start=%d&days=1\r\n",day);
    ESP8266_Send_Cmd(request, "results");
}
/**
  * @brief   阿里云初始 连接阿里云
  * @param   None
  * @retval  None
  * @Note    先烧录ESP8266 MQTT固件库
 **/
void AliYun_Init(void)
{
	//首先，设置MQTT属性 包括阿里云用户名和密码
	while (!ESP8266_Send_Cmd("AT+MQTTUSERCFG=0,1,\"NULL\",\""MQ2_username"\",\""MQ2_passwd"\",0,0,\"\"\r\n", "OK"));
	HAL_Delay(500);
	//然后，设置MQTT客户端ID
	while (!ESP8266_Send_Cmd("AT+MQTTCLIENTID=0,\""MQ2_clientID"\"\r\n", "OK"));
	HAL_Delay(500);
	//最后连接MQTT 服务端
	while (!ESP8266_Send_Cmd("AT+MQTTCONN=0,\""MQ2_mqttHostUrl"\",1883,1\r\n", "OK"));
	HAL_Delay(500);
}
/**
  * @brief   订阅/查询主题 Topic
  * @param   None
  * @retval  None
  * @Note    None
 **/
void AliYun_Topic(void)
{
	while (!ESP8266_Send_Cmd("AT+MQTTSUB=0,\""MQ2_SET"\",1\r\n", "OK"));
	HAL_Delay(500);
//	while (! ESP8266_Send_Cmd("AT+MQTTSUB=0,\""DHT11_POST"\",0\r\n", "OK"));
//	HAL_Delay(500);
}
/**
  * @brief  发布数据到阿里云MQTT主题
  * @param  topic: 目标主题
  * @param  data: 要发送的JSON数据
  * @retval 是否成功
  */
_Bool ESP8266_Publish_Data(const char *topic, const char *data) {
    char cmd[256];
    // 构造AT指令，注意转义引号
    snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",1,0\r\n", topic, data);
    return ESP8266_Send_Cmd(cmd, "OK");
}
/**
  * @brief  复制处理缓冲区数据到data
  * @param  data: 要发送的JSON数据
  * @retval 数据长度
  */
uint16_t esp8266_copy_rxdata(char *data, uint16_t max_len) {
    if (!data || max_len == 0) return 0;

    __disable_irq();  // 防止中断修改数据
    uint16_t valid_len = (ESP01_RX_LEN <= max_len) ? ESP01_RX_LEN : max_len;
    memcpy(data, g_esp8266_rx_buffer, valid_len);
    __enable_irq();

    return valid_len;
}
/**
 * @brief  ESP8266复位 通过拉低RESET再拉高复位
 * @param  NONE
 * @return NONE
 */
void ESP8266_RESET(void)
{
	__HAL_UART_ENABLE_IT(&ESP01_UART_HANDLE, UART_IT_IDLE);
	HAL_UART_Receive_DMA(&ESP01_UART_HANDLE, g_uart2_rx_buffer, ESP01_BUFFER_SIZE);
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port, ESP_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port, ESP_RESET_Pin, GPIO_PIN_SET);
}
void ESP01_IDLE_CallBack(void)
{
	uint8_t idle = __HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE);

	  if(RESET != idle){
	      __HAL_UART_CLEAR_IDLEFLAG(&huart2);
	      HAL_UART_DMAStop(&huart2);


	      __disable_irq();
	      ESP01_RX_LEN = ESP01_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart2.hdmarx);
	      memcpy(g_esp8266_rx_buffer, g_uart2_rx_buffer, ESP01_RX_LEN);
	      __enable_irq();

	      //将串口2接收到的数据发送给串口3，
	      HAL_UART_Transmit(&huart1,g_esp8266_rx_buffer, ESP01_RX_LEN,100);
			
	      memset(g_uart2_rx_buffer,'\0',ESP01_BUFFER_SIZE);
	      HAL_UART_Receive_DMA(&huart2,g_uart2_rx_buffer,ESP01_BUFFER_SIZE);
	      HAL_UART_Receive_DMA(&huart3, g_UART3_Rx_Buffer, ESP01_BUFFER_SIZE);
	  }
}
