#include "main.h"
#include <stdio.h>

static USART_TypeDef *pUSARTx = USART1; // default USART1

#define RX_BUF_SIZE 2048 /* enlarged to hold up to one full 1K YMODEM packet plus latency */
static volatile char rx_buffer[RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

void USART_Select(USART_TypeDef *USARTx)
{
    pUSARTx = USARTx;
}

void USART_Setup(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio = {0};
    USART_InitTypeDef usart = {0};
    NVIC_InitTypeDef nvic = {0};

    if (pUSARTx == USART1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

        // PA9 - TX
        gpio.GPIO_Mode = GPIO_Mode_AF_PP;
        gpio.GPIO_Pin = GPIO_Pin_9;
        gpio.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &gpio);

        // PA10 - RX
        gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        gpio.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOA, &gpio);

        // NVIC
        nvic.NVIC_IRQChannel = USART1_IRQn;
    }
    else if (pUSARTx == USART2)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

        // PA2 - TX
        gpio.GPIO_Mode = GPIO_Mode_AF_PP;
        gpio.GPIO_Pin = GPIO_Pin_2;
        gpio.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &gpio);

        // PA3 - RX
        gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        gpio.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOA, &gpio);

        // NVIC
        nvic.NVIC_IRQChannel = USART2_IRQn;
    }

    usart.USART_BaudRate = baudrate;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(pUSARTx, &usart);
    USART_Cmd(pUSARTx, ENABLE);

    // Enable RX interrupt
    USART_ITConfig(pUSARTx, USART_IT_RXNE, ENABLE);

    // Enable NVIC
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

void USART_SendChar(char c)
{
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
        ;
    USART_SendData(pUSARTx, c);
}

void USART_SendString(const char *str)
{
    while (*str)
    {
        USART_SendChar(*str++);
    }
}

// Non-blocking UART
int USART_Available(void)
{
    return (rx_head != rx_tail);
}

char USART_GetChar(void)
{
    if (rx_head == rx_tail)
        return 0;

    char c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    return c;
}

/* Doc 1 byte voi timeout (ms) */
uint8_t USART_ReadByteTimeout(uint8_t *ch, uint32_t timeout)
{
    uint32_t start = millis();
    while (!USART_Available())
    {
        if ((millis() - start) > timeout)
        {
            return 0; // timeout
        }
    }

    *ch = USART_GetChar();
    return 1;
}

// Interrupt handlers
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        char c = (char)USART_ReceiveData(USART1);
        uint16_t next_head = (rx_head + 1) % RX_BUF_SIZE;
        if (next_head != rx_tail)
        {
            rx_buffer[rx_head] = c;
            rx_head = next_head;
        }
    }
}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        char c = (char)USART_ReceiveData(USART2);
        uint16_t next_head = (rx_head + 1) % RX_BUF_SIZE;
        if (next_head != rx_tail)
        {
            rx_buffer[rx_head] = c;
            rx_head = next_head;
        }
    }
}


/* ----------- printf redirect ------------ */
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

FILE __stdout;

/****************************************************************************************
 * @brief   Ham stub cho system exit
 * 
 * Ham nay duoc yeu cau boi thu vien C chuan. Tren he thong desktop, no thuong dung de 
 * ket thuc chuong trinh va tra ve ma thoat cho he dieu hanh. Tuy nhien tren he thong 
 * nhung (nhu STM32 hoac cac MCU khac) thi khong co he dieu hanh de tra ve.
 * 
 * Do do ham nay chi duoc cai dat nhu mot stub de thoa man linker. Tham so 'x' khong duoc 
 * su dung, chi giu lai de phu hop voi prototype can thiet.
 * 
 * @param   x   Ma thoat (khong su dung trong he thong nhung)
 * 
 * @note    Ham nay khong nen duoc goi truc tiep boi code nguoi dung. No chi duoc goi 
 *          noi bo neu su dung ham 'exit()'.
 ****************************************************************************************/
__attribute__((noreturn, unused)) static void _sys_exit(int x)
{
    (void)x;   /* Tranh canh bao unused parameter */

    /* Vong lap vo han de dung chuong trinh, cho reset */
    while (1) {
        /* Khong lam gi ca */
    }
}

PUTCHAR_PROTOTYPE
{
	(void)f;
	
	if (ch == '\n')
	{
		USART_SendData(pUSARTx, '\r');
		while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	}
	USART_SendData(pUSARTx, (uint8_t)ch);
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	return ch;
}

#define GETCHAR_PROTOTYPE int fgetc(FILE *f)

GETCHAR_PROTOTYPE
{
	(void)f;
	
	while (!USART_Available());
	
	return (int)USART_GetChar();
}
