#include <stdint.h>
#include "lib_message.h"
#include "lib_convert.h"

// Create LUX response
uint8_t Message_Create_Frame_Respond_LUX(uint8_t port, float lux_value, uint8_t *dataout) {
    uint8_t idx = 0;
    dataout[idx++] = 0xAA;
    dataout[idx++] = 0x55;
    dataout[idx++] = (TYPE_MESSAGE_RESPOND_DATA >> 8) & 0xFF;
    dataout[idx++] = TYPE_MESSAGE_RESPOND_DATA & 0xFF;
    dataout[idx++] = (LENGTH_MESSAGE_RESPOND_LUX >> 8) & 0xFF;
    dataout[idx++] = LENGTH_MESSAGE_RESPOND_LUX & 0xFF;
    dataout[idx++] = port;
    dataout[idx++] = SENSOR_LUX;
    uint8_t *fb = Convert_Float_To_Bytes(lux_value);
    for (int i = 3; i >= 0; --i) dataout[idx++] = fb[i];
    uint16_t crc = Check_Sum(dataout, idx, SENSOR_LUX);
    dataout[idx++] = (crc >> 8) & 0xFF;
    dataout[idx++] = crc & 0xFF;
    return idx;
}

// Create DHT11 response (m?i - 4 byte)
uint8_t Message_Create_Frame_Respond_DHT11_Full(
    uint8_t port, uint8_t temp_int, uint8_t temp_dec,
    uint8_t humi_int, uint8_t humi_dec, uint8_t *dataout)
{
    uint8_t idx = 0;
    dataout[idx++] = 0xAA;
    dataout[idx++] = 0x55;
    dataout[idx++] = (TYPE_MESSAGE_RESPOND_DATA >> 8) & 0xFF;
    dataout[idx++] = TYPE_MESSAGE_RESPOND_DATA & 0xFF;
    dataout[idx++] = 0x00;
    dataout[idx++] = 0x0E; // 14 byte t?ng c?ng
    dataout[idx++] = port;
    dataout[idx++] = SENSOR_DHT11;
    dataout[idx++] = humi_int;
    dataout[idx++] = humi_dec;
    dataout[idx++] = temp_int;
    dataout[idx++] = temp_dec;
    uint16_t crc = Check_Sum(dataout, idx, SENSOR_DHT11);
    dataout[idx++] = (crc >> 8) & 0xFF;
    dataout[idx++] = crc & 0xFF;
    return idx;
}

// Parse incoming frame
uint8_t Detect_Frame(uint8_t *datain, frame_Message_t *dataout) {
    uint8_t i = 0;
    dataout->StartFrame = ((uint16_t)datain[i++] << 8) | datain[i++];
    if (dataout->StartFrame != START_BYTES) return 0;
    dataout->TypeMessage = ((uint16_t)datain[i++] << 8) | datain[i++];
    dataout->Length      = ((uint16_t)datain[i++] << 8) | datain[i++];
    uint8_t payload_len = dataout->Length - 2; // tr? CRC
    for (uint8_t j = 0; j < payload_len; ++j) dataout->Data[j] = datain[i++];
    dataout->CheckFrame = ((uint16_t)datain[i++] << 8) | datain[i++];
    uint8_t sensor = dataout->Data[1];
    uint16_t crc = Check_Sum(datain, i - 2, sensor);
    return (crc == dataout->CheckFrame) ? dataout->TypeMessage : 0;
}

// Extract LUX
float Get_Data_Sensor_LUX(frame_Message_t dat) {
    return Convert_Bytes_To_Float(dat.Data[2], dat.Data[3], dat.Data[4], dat.Data[5]);
}

// Extract DHT11 (m?i - 4 byte)
void Get_Data_Sensor_DHT11(frame_Message_t dat,
    uint8_t *humi_int, uint8_t *humi_dec, uint8_t *temp_int, uint8_t *temp_dec)
{
    *humi_int = dat.Data[2];
    *humi_dec = dat.Data[3];
    *temp_int = dat.Data[4];
    *temp_dec = dat.Data[5];
}

// CRC16 for LUX
static uint16_t Check_Sum_LUX(uint8_t *buf, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t p = 0; p < len; ++p) {
        crc ^= buf[p];
        for (uint8_t b = 0; b < 8; ++b) crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0);
    }
    return crc;
}

// Simple sum for DHT11
static uint16_t Check_Sum_DHT11(uint8_t *buf, uint8_t len) {
    uint16_t sum = 0;
    for (uint8_t p = 0; p < len; ++p) sum += buf[p];
    return sum;
}

// Dispatcher
uint16_t Check_Sum(uint8_t *buf, uint8_t len, uint8_t sensor_type) {
    if (sensor_type == SENSOR_LUX)   return Check_Sum_LUX(buf, len);
    if (sensor_type == SENSOR_DHT11) return Check_Sum_DHT11(buf, len);
    return 0;
}
