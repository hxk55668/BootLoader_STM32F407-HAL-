#include "BootLoader.h"
//初始化OTA结构体
OTA_INFOCB OTA_INFO;
//初始化函数指针结构体
load_a LOAD_A;
//初始化更新结构体
UpDataA_CB UpDataA;
//定义一个全集变量标志位 状态机
uint32_t BootStatusFlag;
/**
  * 函    数：读取24C02地址0中的OTA_FLAG信息
  * 参    数：None
  * 返 回 值：None
  */
void EP24C_ReadOTAInFo(void)
{
	memset(&OTA_INFO, 0, OTA_INFOCB_SIZE);
	EP24C_ReadBytes(0, (uint8_t *)&OTA_INFO, OTA_INFOCB_SIZE);
}
/**
  * 函    数：写入24C02地址0中的OTA_FLAG信息
  * 参    数：None
  * 返 回 值：None
  */
void EP24C_WriteOTAInfo(void) {
    uint8_t i;
    uint8_t *wptr = (uint8_t *)&OTA_INFO;
    uint16_t write_size = 16;  // 24C02页大小
    
    for (i = 0; i < OTA_INFOCB_SIZE / write_size; i++) {
        // 每次写入16字节，避免指针大小错误
        EP24C_WriteLongData(i * write_size, wptr + i * write_size, write_size);
        HAL_Delay(5);  // 确保写入周期完成
    }
}
/**
  * 函    数：BootLoader命令行进入函数
  * 参    数：timeout 判断事件
  * 返 回 值：1 进入命令行 0 反之
  */
uint8_t BootLoader_Enter(uint8_t timeout)
{
	U1_Printf("Please, Input w in %dms\r\n", timeout * 1000);
	while(timeout --){
		HAL_Delay(100);
		if (U1_RX_Buffer[0] == 'w'){
			return 1;
		}
	}
	return 0;
}
/**
  * 函    数：编写串口命令行指令
  * 参    数：None
  * 返 回 值：None
  */
void BootLoader_Info(void)
{
	U1_Printf("\r\n");
	U1_Printf("[1]擦除A区\r\n");
	U1_Printf("[2]串口IAP下载A区程序\r\n");
	U1_Printf("[3]设置OTA版本号\r\n");
	U1_Printf("[4]查询OTA版本号\r\n");
	U1_Printf("[5]向外部Flash下载程序\r\n");
	U1_Printf("[6]使用外部Flash内程序\r\n");
	U1_Printf("[7]重启\r\n");
}
/**
  * 函    数：实现串口命令行指令
  * 参    数：Data* 接收的数据 DataLength接收数据长度
  * 返 回 值：None
	* 注    意：功能2中包头oxo1 ACKox06 NCK0x15 EOT0x04
  */
void BootLoader_CMDFunciton(uint8_t *Data, uint16_t DataLength)
{
		int temp;
	    // 去除换行符（如 '\r' 或 '\n'）
    while (DataLength > 0 && (Data[DataLength - 1] == '\r' || Data[DataLength - 1] == '\n')) {
        DataLength--;
    }
		//BootStatusFlag = 0表示没有状态发生
		if (BootStatusFlag == 0){
			if (DataLength == 1 && Data[0] == '1')
			{
					U1_Printf("擦除A区成功\r\n");
					MyFlash_EraseSectors(FLASH_ACode_STARTNUM, FLASH_ACode_NUM);
			}
			else if(DataLength == 1 && Data[0] == '2')
			{
				  U1_Printf("通过Xmodem协议 串口IAP下载A区程序 请使用bin格式文件");
					MyFlash_EraseSectors(FLASH_ACode_STARTNUM, FLASH_ACode_NUM);		//擦除A区
				  BootStatusFlag |= (UPDATA_IAP_XMODEC | UPDATA_IAP_XMODEData); 
					UpDataA.XmodeTimer = 0;
					UpDataA.XmodeNum   = 0;
			}
			else if(DataLength == 1 && Data[0] == '3')
			{
				  U1_Printf("设置OTA版本号\r\n");
				  BootStatusFlag |= OTA_VERSION_FLAG; 
			}
			else if(DataLength == 1 && Data[0] == '4')
			{
				  U1_Printf("查询OTA版本号\r\n");
					EP24C_ReadOTAInFo();
					U1_Printf("版本号:%s", OTA_INFO.OTA_VERSION);
					BootLoader_Info();
			}
			else if(DataLength == 1 && Data[0] == '5')
			{
				  U1_Printf("向外部FLASH下载程序，请输入块编号(1-9)\r\n");
					BootStatusFlag |= CODE_INSTALL_TO_MyFlash; 
			}
			else if(DataLength == 1 && Data[0] == '6')
			{
				  U1_Printf("用外部FLASH程序下载到A区，请输入块编号(1-9)\r\n");
					BootStatusFlag |= CMD6_INSTALL_TO_ASector; 
			}
			else if(DataLength == 1 && Data[0] == '7')
			{
					HAL_Delay(50);
					U1_Printf("重启成功\r\n");
					HAL_NVIC_SystemReset();
			}
	 }
		else if (BootStatusFlag & UPDATA_IAP_XMODEData){									// 判断是否处于处理数据的状态
			U1_Printf("[XMODEM] Recv Len:%d\r\n", DataLength);  			// 打印所有接收长度 调试
			//一次接收133个字节 Data为128个字节 包头为0x01 GITHUB测试1
			if (DataLength == 133 && Data[0] == 0x01){								//数据长度为133 起始位为0x01
				BootStatusFlag &= ~UPDATA_IAP_XMODEC;										//停止发送大写C				
				UpDataA.XmodeCRC = BootLoader_XmodeCRC16(&Data[3], 128);//计算16位CRC
				if (UpDataA.XmodeCRC == Data[131] * 256 + Data[132]){		//比较CRC
					UpDataA.XmodeNum ++;																	//我们是128个数据位一个包 记录接受了接收几个包
					memcpy(&UpDataA.UpDataBuffer[((UpDataA.XmodeNum - 1) % 8) * 128], &Data[3], 128);//将接收到的数据存储在UpDataBuffer中 一次128位 
					if (UpDataA.XmodeNum % 8 == 0){																									 //整数部分8个包 写入Flash	
							if (BootStatusFlag & CMD5_IAP_XModeData){
									//写入外部FLASH 指定块 一次可以写入1024个字节
									// 计算写入地址：块起始地址 + 已写入次数 × 1024
									uint32_t write_addr = UpDataA.XmodeNum * 0x010000;     // 块起始地址
									write_addr += (UpDataA.XmodeNum - 1/ 8) * 1024;           // 偏移量
									Flash_WriteSector(write_addr, &UpDataA.UpDataBuffer[0], UPDATA_SINGLE_SIZE);
							}else{
									MyFlash_WriteBuffer(FLASH_ACode_STARTADDR + ((UpDataA.XmodeNum / 8) - 1) * UPDATA_SINGLE_SIZE, UpDataA.UpDataBuffer, UPDATA_SINGLE_SIZE);
							}
					}
					U1_Printf("\x06");//应答位
					
				}else{
					U1_Printf("\x15");//非应答
				}
			}
			if (DataLength == 1 && Data[0] == 0x04){			//EOT位 发送数据结束位
				U1_Printf("\x06");													//应答
				if (UpDataA.XmodeNum % 8 != 0){							//写入非整数倍
					if (BootStatusFlag & CMD5_IAP_XModeData){
						//写入非整数倍
						// 计算已接收的完整块数
						uint32_t block_count = UpDataA.XmodeNum / 8;
						// 计算在当前块内的写入偏移
						uint32_t flash_offset = block_count * UPDATA_SINGLE_SIZE;
						// 计算绝对写入地址
						uint32_t write_address = UpDataA.XmodeNum * 0x010000 + flash_offset;
						// 写入剩余数据
						MyFlash_WriteBuffer(write_address, UpDataA.UpDataBuffer, (UpDataA.XmodeNum % 8) * 128);
					}
					MyFlash_WriteBuffer(FLASH_ACode_STARTADDR + ((UpDataA.XmodeNum / 8)) * UPDATA_SINGLE_SIZE, UpDataA.UpDataBuffer, (UpDataA.XmodeNum % 8) * 128);
				}
				BootStatusFlag &= ~UPDATA_IAP_XMODEData;		//结束处理数据状态
				HAL_Delay(50);
				HAL_NVIC_SystemReset();
			}
		}
		else if (BootStatusFlag & OTA_VERSION_FLAG){
			//VER-1.0.0-2025/05/23-14:50
			if (DataLength == 26){
				if (sscanf((char *)Data, "VER-%d.%d.%d-%d/%d/%d-%d:%d", &temp, &temp, &temp, &temp, &temp, &temp, &temp, &temp) == 8){
					memset(OTA_INFO.OTA_VERSION, 0, 32);
					memcpy(OTA_INFO.OTA_VERSION, Data, 26);
					EP24C_WriteOTAInfo();
					U1_Printf("版本号设置正确");
					BootStatusFlag &= ~OTA_VERSION_FLAG;
				  BootLoader_Info();
				}else{
					U1_Printf("版本号格式错误\r\n");
				}
			}else U1_Printf("版本号长度设置错误\r\n");
		}
		else if (BootStatusFlag & CODE_INSTALL_TO_MyFlash)
		{
			if (DataLength == 1){
				if (Data[0] >= 0x31 && Data[0] <= 0x39){
					UpDataA.XmodeNum = Data[0] - 0x30;
					BootStatusFlag |= (UPDATA_IAP_XMODEC | UPDATA_IAP_XMODEData | CMD5_IAP_XModeData);
					UpDataA.XmodeTimer = 0;
					UpDataA.XmodeNum   = 0;
					OTA_INFO.Firelen[UpDataA.XmodeNum] = 0;
					Flash_EraseBlockByNumber(UpDataA.XmodeNum);
					U1_Printf("通过Xmodem协议 串口IAP向外部FLASH下载程序（第%d块） 请使用bin格式文件", UpDataA.XmodeNum);
					BootStatusFlag &= ~ CODE_INSTALL_TO_MyFlash;
				}else U1_Printf("输入命令非法\r\n");
			}else U1_Printf("数据长度错误\r\n");
		}
		else if (BootStatusFlag & CMD6_INSTALL_TO_ASector)
		{
			if (DataLength == 1){
				if (Data[0] >= 0x31 && Data[0] <= 0x39){
					UpDataA.XmodeNum = Data[0] - 0x30;
					BootStatusFlag |= UPDATA_UPDTA_A;	
					BootStatusFlag &= ~ CMD6_INSTALL_TO_ASector;
				}else U1_Printf("输入命令非法\r\n");
			}else U1_Printf("数据长度错误\r\n");
		}
}
/**
  * 函    数：BootLoader功能选择函数
  * 参    数：None
  * 返 回 值：None
  * 注    意: 通过判断OTA_FLAG选择更新程序还是跳转程序 并将置位对应状态机的标志位
  */
void BootLoader_Brance(void)
{
    if (BootLoader_Enter(40) == 0) {
        if (OTA_INFO.OTA_FLAG == OTA_SET_FLAG) {
            U1_Printf("OTA SET\r\n");
            BootStatusFlag |= UPDATA_UPDTA_A;
            UpDataA.W25Q128_BlockNum = 0;
        } else {
            U1_Printf("OTA NOT SET,TO Sector A\r\n");
            Load_A(FLASH_ACode_STARTADDR); 
            // 跳转后不应继续执行后续代码
            return; // 关键修复：跳转后直接返回
        }
    } else {
        U1_Printf("BootLoader ENTER CMD\r\n");
        BootLoader_Info();
    }
}
/**
  * 函    数：SP指针赋值函数
  * 参    数：Addr地址
  * 返 回 值：None
  */
__asm void MSR_SP(uint32_t addr)
{
	MSR MSP, r0
	BX r14
}
/**
  * 函    数：无OTA跳转函数
  * 参    数：Addr地址
  * 返 回 值：None
  */
void Load_A(uint32_t addr)
{
	if (*(uint32_t *)addr >= 0x20000000 && *(uint32_t *)addr <= 0x2002FFFF){
		// 关闭所有中断
		__disable_irq();
		MSR_SP(*(uint32_t *)addr);
		LOAD_A = (load_a)*(uint32_t *)(addr + 4);
		BootLoader_Clear();
		LOAD_A();
		// 如果跳转失败，程序会继续执行此处
		U1_Printf("跳转失败！\r\n");
	}else U1_Printf("To Sector A ERROR");
}
void BootLoader_Clear(void)
{
	HAL_UART_DeInit(&huart1);
	HAL_I2C_DeInit(&hi2c1);
}
/**
  * 函    数：实现16位CRC校验
  * 参    数：uint8_t *data uint16_t DataLength
  * 返 回 值：校验值
	* 注    意：uint8_t 表示一位一位的进行校验
  */
uint16_t BootLoader_XmodeCRC16(uint8_t *data, uint16_t DataLength)
{
	uint8_t i = 0;
	uint16_t CRCInit = 0;
	uint16_t CRCPoly = 0x1021;
	
	while (DataLength --)
	{
		CRCInit = (*data << 8) ^ CRCInit;
		for (i = 0; i < 8; i++){
			if (CRCInit & 0x8000)
				CRCInit = (CRCInit << 1) ^ CRCPoly;
			else
				CRCInit = (CRCInit << 1);
		}
		data++;
	}
	return CRCInit;
}
