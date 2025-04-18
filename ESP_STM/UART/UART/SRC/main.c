#include "tim2.h"
#include "uart.h"
#include "bh1750.h"
#include "lib_message.h"
#include "lib_convert.h"

// Prototypes
void GPIOA_Init(void);
void Init(void);
void Loop(void);

int main(void)
{
    Init();
    while (1)
    {
        Loop();  // Vòng l?p chính: g?i d? li?u LUX v? ESP32
    }
}

void GPIOA_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    GPIOA->CRH &= ~(0xF << 28);      // PA15 output
    GPIOA->CRH |= (0x3 << 28);       // General purpose output push-pull
}

void Init(void)
{
    GPIOA_Init();           // LED PA15
    USART1_Init(9600);      // Giao ti?p ESP32
    USART2_Init(9600);      // Giao ti?p PC
    Timer2_Init();          // Blink LED
    BH1750_Init();          // C?m bi?n ánh sáng
}

void Loop(void)
{
    float lux = BH1750_ReadLux();   // Ð?c c?m bi?n ánh sáng

    // Ðóng gói khung g?i sang ESP32
    uint8_t tx_buf[20];
    uint8_t len = Message_Create_Frame_Respond_LUX(PORT_A, lux, tx_buf);
    USART1_Send_Data(tx_buf, len);  // G?i sang ESP32

    // Nháy LED báo dã g?i
    GPIOA->ODR ^= (1 << 15);

    Delay_ms(1000);  // G?i m?i giây
}
