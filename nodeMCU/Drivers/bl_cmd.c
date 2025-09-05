#include "bl_cmd.h"
#include <string.h>

static uint8_t calc_checksum(uint8_t *data, uint16_t len)
{
    uint16_t sum = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

void BL_SendACK(void)
{
    USART_SendChar(CMD_ACK);
}

void BL_SendNACK(void)
{
    USART_SendChar(CMD_NACK);
}

void BL_ProcessCommand(uint8_t *rx_buf, uint16_t len)
{
    if (len < 3)
    { // CMD + LEN + CHECKSUM
        BL_SendNACK();
        return;
    }

    uint8_t cmd = rx_buf[0];
    uint8_t payload_len = rx_buf[1];
    uint8_t *payload = &rx_buf[2];
    uint8_t checksum = rx_buf[2 + payload_len];

    // Verify checksum
    uint8_t calc = calc_checksum(rx_buf, 2 + payload_len);
    if (calc != checksum)
    {
        BL_SendNACK();
        return;
    }

    switch (cmd)
    {
    case CMD_ERASE:
    {
        if (payload_len != 4)
        {
            BL_SendNACK();
            break;
        }
        uint32_t size = 0;
        memcpy(&size, payload, sizeof(uint32_t));
        Flash_Erase(APP_ADDRESS, size);
        BL_SendACK();
        break;
    }

    case CMD_WRITE:
    {
        if (payload_len < 4)
        {
            BL_SendNACK();
            break;
        }
        uint32_t addr = 0;
        memcpy(&addr, payload, sizeof(uint32_t));

        uint8_t *data = &payload[4];
        uint16_t data_len = payload_len - 4;

        if (Flash_Write(addr, data, data_len) == FLASH_OP_OK)
        {
            BL_SendACK();
        }
        else
        {
            BL_SendNACK();
        }
        break;
    }

    case CMD_VERIFY:
    {
        if (payload_len != 6)
        {
            BL_SendNACK();
            break;
        }

        uint16_t checksum_pc = 0;
        uint32_t size_verify = 0;

        memcpy(&checksum_pc, &payload[0], sizeof(uint16_t));
        memcpy(&size_verify, &payload[2], sizeof(uint32_t));

        uint16_t checksum_stm = 0;
        for (uint32_t i = 0; i < size_verify; i++)
        {
            checksum_stm += *(uint8_t *)(APP_ADDRESS + i);
        }
        checksum_stm &= 0xFF;

        if (checksum_stm == checksum_pc)
        {
            BL_SendACK();
        }
        else
        {
            BL_SendNACK();
        }
        break;
    }

    case CMD_JUMP:
    {
        BL_SendACK();
        Delay_Ms(10);
        Jump_To_Application();
        break;
    }

    default:
        BL_SendNACK();
        break;
    }
}
