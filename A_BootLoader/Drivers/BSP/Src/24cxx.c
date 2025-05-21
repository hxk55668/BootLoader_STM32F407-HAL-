#include "24cxx.h"

//超时时间
#define EP24C_TIMEOUT		200
#define EP24C_MEMADD_SIZE 	I2C_MEMADD_SIZE_8BIT
/**
 * @brief   检查设备是否准备好
 * @param   None
 * @retval  HAL_OK or ERROR
 */
HAL_StatusTypeDef EP24C_IsDeviceReady(void) {
	uint32_t Trials = 10;
	return HAL_I2C_IsDeviceReady(&I2C_HANDLE, DEV_ADDR_24CXX, Trials, EP24C_TIMEOUT);
}
/**
 * @brief   指定地址写入一个字节
 * @param   memAddress指定的地址 byteData一字节数据
 * @retval  HAL_OK or ERROR
 */
HAL_StatusTypeDef EP24C_WriteOneByte(uint16_t memAddress, uint8_t byteData) {
	return HAL_I2C_Mem_Write(&I2C_HANDLE, DEV_ADDR_24CXX, memAddress,
			EP24C_MEMADD_SIZE, &byteData, 1, EP24C_TIMEOUT);
}
/**
 * @brief   指定地址读取一个字节
 * @param   memAddress指定的地址 byteData读取的数据
 * @retval  HAL_OK or ERROR
 */
HAL_StatusTypeDef EP24C_ReadOneByte(uint16_t memAddress, uint8_t *byteData) {
	return HAL_I2C_Mem_Read(&I2C_HANDLE, DEV_ADDR_24CXX, memAddress,
				EP24C_MEMADD_SIZE, byteData, 1, EP24C_TIMEOUT);
}
/**
 * @brief   指定地址读取多个字节
 * @param   memAddress指定的地址  *pBuffer读取数据存储的指针 bufferLen读取的长度
 * @retval  HAL_OK or ERROR
 */
HAL_StatusTypeDef EP24C_ReadBytes(uint16_t memAddress, uint8_t *pBuffer, uint16_t bufferLen) {
	if (bufferLen > MEM_SIZE_24CXX) {
		return HAL_ERROR;
	}
	return HAL_I2C_Mem_Read(&I2C_HANDLE, DEV_ADDR_24CXX, memAddress,
				EP24C_MEMADD_SIZE, pBuffer, bufferLen, EP24C_TIMEOUT);
}
/**
 * @brief   指定地址写入一页 不会超过一页
 * @param   memAddress指定的地址 *pBuffer写入的数据 bufferLen写入的长度
 * @retval  HAL_OK or ERROR
 */
HAL_StatusTypeDef EP24C_WriteInOnePage(uint16_t memAddress, uint8_t *pBuffer, uint16_t bufferLen) {
	if (bufferLen > PAGE_SIZE_24CXX) {
		return HAL_ERROR;
	}
	return HAL_I2C_Mem_Write(&I2C_HANDLE, DEV_ADDR_24CXX, memAddress,
				EP24C_MEMADD_SIZE, pBuffer, bufferLen, EP24C_TIMEOUT);
}
/**
 * @brief   指定地址写入超一页地址
 * @param   memAddress指定的地址 *pBuffer写入的数据 bufferLen写入的长度
 * @retval  HAL_OK or ERROR
 */
HAL_StatusTypeDef EP24C_WriteLongData(uint16_t memAddress, uint8_t *pBuffer, uint16_t bufferLen) {
	if (bufferLen > MEM_SIZE_24CXX) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef result = HAL_ERROR;
	if (bufferLen <= PAGE_SIZE_24CXX) {
		return HAL_I2C_Mem_Write(&I2C_HANDLE, DEV_ADDR_24CXX, memAddress,
						EP24C_MEMADD_SIZE, pBuffer, bufferLen, EP24C_TIMEOUT);
	}

	uint8_t *pt = pBuffer;
	uint16_t pageCount = bufferLen / PAGE_SIZE_24CXX;
	for (uint16_t i = 0; i < pageCount; ++i) {
		result = HAL_I2C_Mem_Write(&I2C_HANDLE, DEV_ADDR_24CXX, memAddress,
								EP24C_MEMADD_SIZE, pt, PAGE_SIZE_24CXX, EP24C_TIMEOUT);
		pt += PAGE_SIZE_24CXX;
		memAddress += PAGE_SIZE_24CXX;
		HAL_Delay(5);

		if (result != HAL_OK) {
			return result;
		}
	}

	uint8_t leftBytes = bufferLen % PAGE_SIZE_24CXX;
	if (leftBytes > 0) {
		result = HAL_I2C_Mem_Write(&I2C_HANDLE, DEV_ADDR_24CXX, memAddress,
										EP24C_MEMADD_SIZE, pt, leftBytes, EP24C_TIMEOUT);
	}
	return result;
}
