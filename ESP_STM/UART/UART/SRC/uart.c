#include "uart.h"
#include <stdio.h>

// --- USART1 --- (UART1 giao ti?p v?i ESP32)
void USART1_Init(uint32_t baud_rate)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    
    // PA9 TX (AF push-pull)
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |= (0xB << 4);  // 50 MHz, AF Push-Pull

    // PA10 RX (floating input)
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |= (0x4 << 8);  // Floating input

    // C?u hình baud rate
    USART1->BRR = (uint16_t)(72000000 / baud_rate);
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;  // Enable TX, RX, and interrupt
    NVIC_EnableIRQ(USART1_IRQn);  // Kích ho?t ng?t USART1
}

void USART1_Send_Char(char chr)
{
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (uint8_t)chr;
}

void USART1_Send_String(const char* str)
{
    while (*str) USART1_Send_Char(*str++);
}

void USART1_Send_Data(uint8_t* data, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++) {
        USART1_Send_Char(data[i]);
    }
}

// --- USART2 --- (UART2 giao ti?p v?i PC)
void USART2_Init(uint32_t baud_rate)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // PA2 TX (AF push-pull)
    GPIOA->CRL &= ~(0xF << 8);
    GPIOA->CRL |= (0xB << 8);  // 50 MHz, AF Push-Pull

    // PA3 RX (floating input)
    GPIOA->CRL &= ~(0xF << 12);
    GPIOA->CRL |= (0x4 << 12);  // Floating input

    // C?u hình baud rate
    USART2->BRR = (uint16_t)(36000000 / baud_rate);
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;  // Enable TX, RX
}

void USART2_Send_Char(char chr)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint8_t)chr;
}

void USART2_Send_String(const char* str)
{
    while (*str) USART2_Send_Char(*str++);
}

void USART2_Send_Data(uint8_t* data, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++) {
        USART2_Send_Char(data[i]);
    }
}

// --- X? lý ng?t USART1 (nh?n d? li?u t? ESP32) ---
void USART1_IRQHandler(void)
{
    static uint8_t rx_buf[256];
    static uint8_t rx_idx = 0;

    if (USART1->SR & USART_SR_RXNE)
    {
        uint8_t byte = (uint8_t)USART1->DR;

        if (rx_idx < sizeof(rx_buf)) {
            rx_buf[rx_idx++] = byte;  // Luu byte vào buffer
        }

        // Ki?m tra n?u dã nh?n d? d? li?u
        if (rx_idx >= 6) {
            uint16_t length_field = ((uint16_t)rx_buf[4] << 8) | rx_buf[5]; // Ki?m tra chi?u dài frame
            uint16_t frame_size =length_field; // T?ng kích thu?c c?a frame (bao g?m header + d? li?u)

            // N?u dã nh?n d? m?t frame
            if (rx_idx >= frame_size) {
                // Ki?m tra n?u frame b?t d?u v?i AA 55 (Header)
                if (rx_buf[0] == 0xAA && rx_buf[1] == 0x55) {
                    // In toàn b? frame g?c ra UART2 d? ki?m tra
                    USART2_Send_String("RF:  ");
                    for (uint8_t i = 0; i < rx_idx; i++) {
                        char hex[6];
                        snprintf(hex, sizeof(hex), "%02X ", rx_buf[i]);
                        USART2_Send_String(hex);
                    }
                    USART2_Send_String("\r\n");

                    // Phân tích d? li?u DHT11 t? frame
                   uint8_t port = rx_buf[6];        // C?ng truy?n d? li?u
									 uint8_t sensor_type = rx_buf[7]; // Lo?i c?m bi?n (DHT11)

									 uint8_t humidity_int = rx_buf[8];    // Ð? ?m nguyên
									 uint8_t humidity_dec = rx_buf[9];    // Ð? ?m ph?n th?p phân

									 uint8_t temperature_int = rx_buf[10];  // Nhi?t d? nguyên
									 uint8_t temperature_dec = rx_buf[11]; // Nhi?t d? ph?n th?p phân

										// Ki?m tra d? ?m và nhi?t d?
									float humidity = humidity_int + (humidity_dec / 10.0);
									float temperature = temperature_int + (temperature_dec / 10.0);

										// In thông tin nh?n du?c ra UART2
									USART2_Send_String("Received DHT11 Data: ");
									char buffer[64];
									snprintf(buffer, sizeof(buffer), "Humidity: %.1f, Temperature: %.1f",humidity, temperature);
									USART2_Send_String(buffer);
									USART2_Send_String("\r\n");

                    // Reset buffer d? chu?n b? nh?n frame ti?p theo
                    rx_idx = 0; // Ch? reset rx_idx sau khi x? lý thành công m?t frame
                } else {
                    // N?u không ph?i là frame h?p l?, reset buffer và b? qua
                    rx_idx = 0;
                }
            }
        }
    }
}







