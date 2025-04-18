#ifndef __UART_H__
#define __UART_H__

#include "stm32f10x.h"
#include <stdint.h>

// USART1 <-> ESP32
void USART1_Init(uint32_t baud_rate);
void USART1_Send_Char(char chr);
void USART1_Send_String(const char* str);
void USART1_Send_Data(uint8_t* data, uint8_t length);
void USART1_IRQHandler(void);

// USART2 <-> PC/Lab terminal
void USART2_Init(uint32_t baud_rate);
void USART2_Send_Char(char chr);
void USART2_Send_String(const char* str);
void USART2_Send_Data(uint8_t* data, uint8_t length);

#endif // __UART_H__
