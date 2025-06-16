#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include "main.h"
#include "stm32f4xx_hal.h"
#include "U1_BootLoader.h"
#include "My_Flash.h"
#include "24cxx.h"
#include "w25flash.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>    // 标准整数类型定义（如uint32_t）
#include "esp01_uart.h"
//Flash私有定义
//F407FLASH一共12个扇区 前四个扇区为16K 第五个为64K 后面七个为128K
#define F407_FLASH_STARTADDR				0x08000000
#define F407_FLASH_16SIZE						16  * 1024
#define F407_FLASH_64SIZE						64  * 1024
#define F407_FLASH_128SIZE					128 * 1024
#define FLASH_SECTOR_NUM						12
#define FLASH_BOOTLOADER_NUM				4
#define FLASH_ACode_NUM							FLASH_SECTOR_NUM - FLASH_BOOTLOADER_NUM
#define FLASH_ACode_STARTNUM				FLASH_BOOTLOADER_NUM											//B区扇区为0 1 2 3则B区起始就为4
#define FLASH_ACode_STARTADDR				F407_FLASH_STARTADDR + F407_FLASH_16SIZE * FLASH_ACode_STARTNUM 
#define OTA_SET_FLAG								0xAABB1122
#define UPDATA_SINGLE_SIZE					1024																			//一次更新大小 一点一点更新 一次1024
#define UPDATA_UPDTA_A							0x00000001																//更新事件状态位
#define UPDATA_IAP_XMODEC						0x00000002																//IAP下载标志位
#define UPDATA_IAP_XMODEData				0x00000004																//处理IAP数据的标志位
#define OTA_VERSION_FLAG						0x00000008																//设置版本号OTAFlag
#define CODE_INSTALL_TO_MyFlash			0x00000010																//
#define CMD5_IAP_XModeData					0x00000020
#define CMD6_INSTALL_TO_ASector			0x00000040
//OTA结构体定义 这些信息都存放到24C02中
typedef struct{
	uint32_t OTA_FLAG;
	uint32_t Firelen[11];							//0号成员固定对应OTA的大小
	uint8_t  OTA_VERSION[32];					//24c02中用于存放版本号
}OTA_INFOCB;
//更新事件结构体
typedef struct{
	uint8_t 	UpDataBuffer[UPDATA_SINGLE_SIZE];
	uint32_t	W25Q128_BlockNum;				//用于记录用于更新那个程序，外部FLASH存储有多个程序 需要那个程序就更新到A区
	uint32_t  XmodeTimer;
	uint32_t  XmodeNum;
	uint32_t  XmodeCRC;
}UpDataA_CB;
//函数指针 用于指向PC
typedef void (*load_a)(void);


void BootLoader_Brance(void);
__asm void MSR_SP(uint32_t addr);
void Load_A(uint32_t addr);
void BootLoader_Clear(void);
void EP24C_WriteOTAInfo(void);
void EP24C_ReadOTAInFo(void);
void BootLoader_CMDFunciton(uint8_t *Data, uint16_t DataLength);
uint16_t BootLoader_XmodeCRC16(uint8_t *data, uint16_t DataLength);
void BootLoader_Info(void);
//外部声明
#define OTA_INFOCB_SIZE							sizeof(OTA_INFOCB)
extern OTA_INFOCB OTA_INFO;
extern load_a LOAD_A;
extern UpDataA_CB UpDataA;
extern uint32_t BootStatusFlag;

#endif
