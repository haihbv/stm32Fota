#include "main.h"

#define FLASH_PAGE_SIZE 1024 // STM32F103C8: 1KB/page; high-density STM32F1: 2KB/page
#define FLASH_BASE_ADDR 0x08000000
#define FLASH_TOTAL_SIZE (64 * 1024) // 64KB flash

/**
 * @brief   Xóa toàn bộ vùng Flash từ startAddr với độ dài size
 * @param   startAddr   địa chỉ bắt đầu (phải thuộc vùng flash)
 * @param   size        số byte cần xóa
 * @retval  FlashStatus
 */
FlashStatus Flash_Erase(uint32_t startAddr, uint32_t size)
{
	if (startAddr < FLASH_BASE_ADDR || startAddr >= (FLASH_BASE_ADDR + FLASH_TOTAL_SIZE))
	{
		return FLASH_OP_ADDR_INVALID;
	}

	uint32_t endAddr = startAddr + size;
	if (endAddr > (FLASH_BASE_ADDR + FLASH_TOTAL_SIZE))
	{
		endAddr = FLASH_BASE_ADDR + FLASH_TOTAL_SIZE;
	}

	FLASH_Unlock();

	for (uint32_t addr = startAddr; addr < endAddr; addr += FLASH_PAGE_SIZE)
	{
		FLASH_Status st = FLASH_ErasePage(addr);
		if (st != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return FLASH_OP_ERROR;
		}
	}

	FLASH_Lock();
	return FLASH_OP_OK;
}

/**
 * @brief   Ghi dữ liệu vào Flash
 * @param   addr    địa chỉ flash cần ghi (phải chẵn)
 * @param   data    buffer dữ liệu
 * @param   len     số byte cần ghi
 * @retval  FlashStatus
 */
FlashStatus Flash_Write(uint32_t addr, uint8_t *data, uint16_t len)
{
	if (addr % 2 != 0)
	{
		return FLASH_OP_ADDR_INVALID;
	}
	if (addr < FLASH_BASE_ADDR || addr >= (FLASH_BASE_ADDR + FLASH_TOTAL_SIZE))
	{
		return FLASH_OP_ADDR_INVALID;
	}

	FLASH_Unlock();

	for (uint16_t i = 0; i < len; i += 2)
	{
		uint8_t b1 = data[i];
		uint8_t b2 = (i + 1 < len) ? data[i + 1] : 0xFF;
		uint16_t halfword = (uint16_t)(b1 | (b2 << 8));

		FLASH_Status st = FLASH_ProgramHalfWord(addr + i, halfword);
		if (st != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return FLASH_OP_ERROR;
		}

		// Verify lại dữ liệu sau khi ghi
		if (*(volatile uint16_t *)(addr + i) != halfword)
		{
			FLASH_Lock();
			return FLASH_OP_ERROR;
		}
	}

	FLASH_Lock();
	return FLASH_OP_OK;
}

/**
 * @brief   Đọc dữ liệu từ Flash vào buffer
 * @param   addr    địa chỉ flash
 * @param   buf     buffer đích
 * @param   len     số byte cần đọc
 */
void Flash_ReadBuffer(uint32_t addr, uint8_t *buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		buf[i] = *(volatile uint8_t *)(addr + i);
	}
}
