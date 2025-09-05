/**
******************************************************************************
* @file    main.c
* @author  haihbv
* @date    5/9/2025
* @brief   Bootloader: cho Host(PC) gui ban tin xuong Node
******************************************************************************
*/

/****************************************************************************************
 * Includes
 ****************************************************************************************/
 #include "main.h"
 #include <stdio.h>
 
 /****************************************************************************************
 * Prototypes
 ****************************************************************************************/
static void GPIOA_Init(void);

/****************************************************************************************
 * Setup, Loop
 ****************************************************************************************/
static void Setup(void);
static void Loop(void);

int main(void)
{
	Setup();
	while (1)
	{
		Loop();
	}
}

/****************************************************************************************
 * API
 ****************************************************************************************/
static void GPIOA_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* Led Off */
	GPIO_ResetBits(GPIOA, GPIO_Pin_0); 
}

static void Setup()
{
	SystemInit();
	Delay_Init();
	USART_Setup(115200);
	GPIOA_Init();
	
	printf("Booloader started... waiting for message from Host\r\n");
}

static void Loop()
{
	/* Blink LED moi 100ms bao hieu van dang o trong bootloader */
	static uint32_t start = 0;
	if (millis() - start >= 234)
	{
		GPIOA->ODR ^= GPIO_Pin_0;
		start = millis();
	}
	
	/* Nhan du lieu tu UART */
	static uint8_t rx_buff[256];
	static uint16_t rx_len = 0;
	
	while (USART_Available())
	{
		char c = USART_GetChar();
		rx_buff[rx_len++] = (uint8_t)c; // luu ky tu nhan duoc vao rx_buff
		
		/* Packed du CMD + LEN + PAYLOAD + CHECKSUM */
		if (rx_len >= 2)
		{
			uint8_t expected_len = 2 + rx_buff[1] + 1;
			if (rx_len >= expected_len)
			{
				BL_ProcessCommand(rx_buff, expected_len);
				rx_len = 0;
			}
		}
		
		/* Reset neu tran buffer */
		if (rx_len >= sizeof(rx_buff))
		{
			rx_len = 0;
		}
	}
}

