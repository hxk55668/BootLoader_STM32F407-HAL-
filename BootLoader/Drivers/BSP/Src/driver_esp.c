#include "driver_esp.h"



uint8_t WIFI_Name_Cipher[]="AT+CWJAP=\"Redmi K40\",\"123456789\"\r\n";

#define ESP01_UART_HANDLE	huart2

void esp01_uart_send_data(uint8_t *data,uint16_t data_len)
{
	HAL_UART_Transmit(&ESP01_UART_HANDLE, data , data_len,100 );
}

/*****************************************上面是需要修改的地方******************************************************/
/*私有定义*/
#define ESP8266_BUFFER_SIZE 				1024
uint8_t g_esp8266_rx_buffer[ESP8266_BUFFER_SIZE];   // ESP8266响应数据接收缓冲区
/*****************************************函数******************************************************/

/**
 * @brief  ESP8266发送命令
 * @param  cmd为AT指令，res为返回值
 * @return 成功返回1，失败返回0
 * 通过串口2发送AT指令给esp-01s模块，如果检索到串口2收到esp-01s回复的字符中存在指定字符（res），则表示成功且返回1，否则失败返回0
  *其中cmd是需要发送的AT指令，res是esp-01s模块回复（发送）给串口2的字符消息
 */
_Bool ESP8266_Send_Cmd(char *cmd, char *res)
{
	uint8_t TimeOut = 20;
	esp01_uart_send_data(cmd, strlen(cmd));
	while (TimeOut --)
	{
		if (strstr(g_esp8266_rx_buffer, res) != NULL)
			return 1; //Success
		HAL_Delay(100);
	}
	return 0; //Fail
}
/**
  * @brief   测试命名
  * @param   None
  * @retval  None
 **/
_Bool ESP8266_TEST(void)
{
	return ESP8266_Send_Cmd("AT\r\n","OK");
}
/**
  * @brief   选择ESP8266的工作模式
  * @param   enumMode 模式类型 STA AP STA_AP
  * @retval  返回 true 发送成功，false 失败
 **/
_Bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
	switch (enumMode)
	{
		case STA:
			return ESP8266_Send_Cmd("AT+CWMODE=1\r\n", "OK");
		case AP:
			return ESP8266_Send_Cmd("AT+CWMODE=2\r\n", "OK");
		case STA_AP:
			return ESP8266_Send_Cmd("AT+CWMODE=3\r\n", "OK");
		default:
			return 0;
	}
}
/**
 * @brief  设置ESP8266的连接模式（单连接/多连接）
 * @param  mode 0=单连接模式，1=多连接模式
 * @return 成功返回1，失败返回0
 */
_Bool ESP8266_Set_MultiplesMode(uint8_t mode)
{
	char cmd[32];
	//构造AT指令：AT+CIPMUX=<mode>\r\n
	sprintf(cmd, "AT+CIPMUX=%d\r\n", mode);
	return ESP8266_Send_Cmd(cmd, "OK");
}
/**
 * @brief  连接WIFI
 * @param  None
 * @return 成功返回1，失败返回0
 * @Note   注意修改WIFI_Name_Cipher
 */
_Bool ESP8266_JoinAP(void)
{
	return ESP8266_Send_Cmd(WIFI_Name_Cipher,"OK");
}
/**
 * @brief  连接TCP服务器（自动适配单连接/多连接模式）
 * @param  id    连接ID（仅在多连接模式下有效，单连接模式传0）
 * @param  ip    服务器IP地址（如 "192.168.1.100"）
 * @param  port  服务器端口号（如 8080）
 * @return 成功返回1，失败返回0
 */
_Bool ESP8266_Connect_TCPServer(uint8_t id, const char* ip, uint16_t port) {
    char cmd[128];
    // 根据当前模式构造AT指令
    if (strstr(g_esp8266_rx_buffer, "+CIPMUX:1") != NULL) {
        // 多连接模式：AT+CIPSTART=<id>,"TCP","<ip>",<port>\r\n
        sprintf(cmd, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n", id, ip, port);
    } else {
        // 单连接模式：AT+CIPSTART="TCP","<ip>",<port>\r\n
        sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ip, port);
    }
    return ESP8266_Send_Cmd(cmd, "OK");
}
/**
  * @brief  创建TCP服务器
  * @param  port 服务器端口号
  * @retval 成功返回1，失败返回0
  */
_Bool ESP8266_Create_TCPServer(uint16_t port)
{
    // 1.设置多连接模式
    if(!ESP8266_Set_MultiplesMode(1)) return 0;

    // 2.发送创建服务器命令
    char cmd[32];
    sprintf(cmd, "AT+CIPSERVER=1,%d\r\n", port);
    return ESP8266_Send_Cmd(cmd, "OK");
}
/**
  * @brief  ESP8266发送数据
  * @param  str 数据指针
  * @param  uid 连接ID（单连接模式填0）
  * @param  strlen 数据长度
  * @retval 成功返回1，失败返回0
  */
_Bool ESP8266_Send_Str(char* str, uint8_t uid, uint16_t length)
{
    char cmd[32];

    // 1.构造发送命令（自动适配单/多连接模式）
    if(uid != 0) {
        sprintf(cmd, "AT+CIPSEND=%d,%d\r\n", uid, length);
    } else {
        sprintf(cmd, "AT+CIPSEND=%d\r\n", length);
    }

    // 2.发送命令并等待">"提示符
    esp01_uart_send_data((uint8_t*)cmd, strlen(cmd));

    uint8_t timeout = 20;
    while(timeout--) {
        if(strstr((char*)g_esp8266_rx_buffer, "> ") != NULL) break;
        HAL_Delay(100);
    }
    if(timeout == 0) return 0;

    // 3.清空接收缓冲区
    memset(g_esp8266_rx_buffer, 0, sizeof(g_esp8266_rx_buffer));

    // 4.发送实际数据
    esp01_uart_send_data((uint8_t*)str, length);

    // 5.等待发送确认
    timeout = 20;
    while(timeout--) {
        if(strstr((char*)g_esp8266_rx_buffer, "SEND OK") ||
           strstr((char*)g_esp8266_rx_buffer, "OK")) {
            return 1;
        }
        HAL_Delay(100);
    }
    return 0;
}
/**
  * @brief  建立SNTP服务器x`
  * @param  None
  * @retval None
  */
_Bool ESP8266_Set_SNTP_Config(void)
{
    // 构造完整AT指令（注意转义引号）
    char cmd[] = "AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n";

    // 发送命令并等待"OK"响应
    return ESP8266_Send_Cmd(cmd, "OK");
}
/**
 * @brief 激活ESP-01S透传模式（需在已建立TCP连接后调用）
 * @return 成功返回1，失败返回0
 * @note 执行流程：设置透传模式 → 激活透传
 */
_Bool ESP8266_Enable_Passthrough(void) {
    while(!ESP8266_Send_Cmd("AT+CIPMODE=1\r\n", "OK")) HAL_Delay(100);
    // 2. 激活透传（直接返回状态）
    return ESP8266_Send_Cmd("AT+CIPSEND\r\n", ">");
}

