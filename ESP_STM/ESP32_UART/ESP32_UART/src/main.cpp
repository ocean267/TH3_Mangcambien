#include <Arduino.h>
#include "lib_convert.h"
#include "lib_message.h"
#include <DHT.h>

// UART buffer
#define BUFFER_SIZE       20
#define HEADER_1          0xAA
#define HEADER_2          0x55
#define MESSAGE_LENGTH    14  // độ dài bản tin LUX

// DHT11 config
#define DHT_PIN           4    // chân nối DHT11
#define DHT_TYPE          DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Nhận và xử lý frame LUX
void processMessage(uint8_t* data) {
    float lux = Convert_Bytes_To_Float(data[11], data[10], data[9], data[8]);
    Serial.print("LUX: ");
    Serial.println(lux, 2);
}

// Nhận sensor LUX và gửi DHT11 về STM
void loop_lux_sensor(void) {
    static uint8_t buffer[BUFFER_SIZE];
    static uint8_t idx = 0;

    while (Serial2.available()) {
        uint8_t b = Serial2.read();
        buffer[idx++] = b;

        // Kiểm tra header
        if (idx >= 2 && !(buffer[0] == HEADER_1 && buffer[1] == HEADER_2)) {
            if (buffer[idx - 1] == HEADER_1) {
                buffer[0] = HEADER_1;
                idx = 1;
            } else {
                idx = 0;
            }
            continue;
        }

        // Nếu đủ length, xử lý
        if (idx == MESSAGE_LENGTH) {
            Serial.println("\nComplete LUX message:");
            for (uint8_t i = 0; i < MESSAGE_LENGTH; i++) {
                Serial.print(buffer[i], HEX);
                Serial.print(' ');
            }
            Serial.println();

            // Xử lý dữ liệu LUX
            processMessage(buffer);

            // --- Đọc DHT11 và gửi về STM ---
            float h = dht.readHumidity();
            float t = dht.readTemperature();
            if (isnan(h) || isnan(t)) {
                Serial.println("Failed to read DHT11");
            } else {
                uint8_t temp_int = (uint8_t)t;
                uint8_t temp_dec = (uint8_t)((t - temp_int) * 10.0);
                uint8_t humi_int = (uint8_t)h;
                uint8_t humi_dec = (uint8_t)((h - humi_int) * 10.0);

                uint8_t frame_dht[20];
                uint8_t len = Message_Create_Frame_Respond_DHT11_Full(
                    PORT_A, temp_int, temp_dec, humi_int, humi_dec, frame_dht
                );

                Serial.print("Sending DHT11 frame: ");
                for (uint8_t i = 0; i < len; i++) {
                    Serial.print(frame_dht[i], HEX);
                    Serial.print(' ');
                }
                Serial.println();

                Serial2.write(frame_dht, len);
            }

            idx = 0; // Reset buffer
        }

        if (idx >= BUFFER_SIZE) idx = 0;
    }
}

void setup() {
    Serial.begin(9600);
    Serial2.begin(9600);
    dht.begin();
    Serial.println("Start UART+DHT11");
}

void loop() {
    loop_lux_sensor();
}
