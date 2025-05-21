#include "Task_Main.h"

/**
  * 函    数：函数初始化，所有初始化代码都放在这
  * 参    数：None
  * 返 回 值：None
  */
void Task_Main_Init(void)
{
	U1BootLoader_Init();
}
/**
  * 函    数：主函数
  * 参    数：None
  * 返 回 值：None
  */
void Task_Main(void) {
	
    EP24C_WriteOTAInfo();
    // 从24C02读取
    EP24C_ReadOTAInFo();
    BootLoader_Brance();
}
/**
  * 函    数：主函数中的循环,所有需要循环的代码放在这
  * 参    数：None
  * 返 回 值：None
  * 注    意：我们在循环中判断状态机的标志 不同的标志对应不同的功能 我们这里测试的是OTA更新事件功能  
  */
void Task_Main_While(void)
{
	HAL_Delay(10);
	uint8_t i;
	if(UCB_CB.URxDataOut != UCB_CB.URxDataIn){
		for (i = 0; i < UCB_CB.URxDataOut -> end - UCB_CB.URxDataOut ->start +1; i++){
			U1_Printf("%c\r\n", UCB_CB.URxDataOut->start[i]);
		}
		BootLoader_CMDFunciton(UCB_CB.URxDataOut ->start, UCB_CB.URxDataOut -> end - UCB_CB.URxDataOut ->start +1);
		UCB_CB.URxDataOut++;
		if (UCB_CB.URxDataOut == UCB_CB.URxDataEnd)
		{
			UCB_CB.URxDataOut = &UCB_CB.URxDataBuffer[0];
		}
	}
	//表示IAP下载状态发生
	if (BootStatusFlag & UPDATA_IAP_XMODEC){
	  if (UpDataA.XmodeTimer >= 100){
			UpDataA.XmodeTimer = 0;
			U1_Printf("C");
		}	
		UpDataA.XmodeTimer ++;
	}
	
	//表示测试给OTA更新事件
	if (BootStatusFlag & UPDATA_UPDTA_A){
		//更新A区 OTA事件更新的长度存储在Firelen的0号中
		U1_Printf("本次更新的长度为：%d\r\n",OTA_INFO.Firelen[UpDataA.W25Q128_BlockNum]);
		//STM32的Flash控制器对写入操作有严格的 对齐限制 最小写入4个字节
		if (OTA_INFO.Firelen[UpDataA.W25Q128_BlockNum] % 4 == 0){
			//先擦除A区 A区起始为第四个扇区 4-11一共八个扇区
			MyFlash_EraseSectors(FLASH_ACode_STARTNUM, FLASH_ACode_NUM);
			//判断1024的倍数  拿数据 写入FLASH
			for(i = 0;i < OTA_INFO.Firelen[UpDataA.W25Q128_BlockNum] / UPDATA_SINGLE_SIZE; i++){
				//从W25Q128的指定BLOCK读取数据 
				Flash_ReadBytes(1024 * i + UpDataA.W25Q128_BlockNum * 64 * 1024, UpDataA.UpDataBuffer, UPDATA_SINGLE_SIZE);
				//拿完数据后 更新到A区 写整数
				MyFlash_WriteBuffer(i * 1024 + FLASH_ACode_STARTADDR, UpDataA.UpDataBuffer, UPDATA_SINGLE_SIZE);
			}
			if (OTA_INFO.Firelen[UpDataA.W25Q128_BlockNum] % 1024 != 0){	//写余数
				Flash_ReadBytes(1024 * i + UpDataA.W25Q128_BlockNum * 64 * 1024, UpDataA.UpDataBuffer, OTA_INFO.Firelen[UpDataA.W25Q128_BlockNum] % 1024);
				//拿完数据后 更新到A区
				MyFlash_WriteBuffer(i * 1024 + FLASH_ACode_STARTADDR, UpDataA.UpDataBuffer, OTA_INFO.Firelen[UpDataA.W25Q128_BlockNum] % 1024);
			}
			if (UpDataA.W25Q128_BlockNum == 0){
				OTA_INFO.OTA_FLAG = 0;
				EP24C_WriteOTAInfo();
			}
			HAL_NVIC_SystemReset();
		}else{
			U1_Printf("本次写入长度有误ERROR");
			//清除标志位
			BootStatusFlag &=~ UPDATA_UPDTA_A;
		}
	}	
}







