#include <stdint.h>
#include "lib_message.h"
#include "lib_convert.h"


uint8_t Message_Create_Frame_Respond_LUX(uint8_t port, float lux_value, uint8_t *dataout) {
    uint8_t count = 0;
    // Start bytes
    dataout[count++] = 0xAA;
    dataout[count++] = 0x55;
    // Message type
    dataout[count++] = (TYPE_MESSAGE_RESPOND_DATA >> 8) & 0xFF;
    dataout[count++] = TYPE_MESSAGE_RESPOND_DATA & 0xFF;
    // Length
    dataout[count++] = (LENGTH_MESSAGE_RESPOND_LUX >> 8) & 0xFF;
    dataout[count++] = LENGTH_MESSAGE_RESPOND_LUX & 0xFF;
    // Port + sensor
    dataout[count++] = port;
    dataout[count++] = SENSOR_LUX;
    // Payload: float BE
    uint8_t *b = Convert_Float_To_Bytes(lux_value);
    for (int i = 3; i >= 0; --i) dataout[count++] = b[i];
    // CRC
    uint16_t crc = Check_Sum(dataout, count, SENSOR_LUX);
    dataout[count++] = (crc >> 8) & 0xFF;
    dataout[count++] = crc & 0xFF;
    return count;
}

uint8_t Message_Create_Frame_Respond_DHT11_Full(
    uint8_t port, uint8_t temp_int, uint8_t temp_dec,
    uint8_t humi_int, uint8_t humi_dec, uint8_t *dataout)
{
    uint8_t count = 0;
    dataout[count++] = 0xAA;
    dataout[count++] = 0x55;
    dataout[count++] = (TYPE_MESSAGE_RESPOND_DATA >> 8) & 0xFF;
    dataout[count++] = TYPE_MESSAGE_RESPOND_DATA & 0xFF;

    dataout[count++] = 0x00;
    dataout[count++] = 0x0E;  // Length = 14 bytes (6 + 2 + 4 + 2 CRC)

    dataout[count++] = port;
    dataout[count++] = SENSOR_DHT11;

    // Độ ẩm và nhiệt độ
    dataout[count++] = humi_int; // Độ ẩm nguyên
    dataout[count++] = humi_dec; // Độ ẩm phần thập phân

    dataout[count++] = temp_int; // Nhiệt độ nguyên
    dataout[count++] = temp_dec; // Nhiệt độ phần thập phân

    // Tính CRC
    uint16_t crc = Check_Sum(dataout, count, SENSOR_DHT11);
    dataout[count++] = (crc >> 8) & 0xFF;
    dataout[count++] = crc & 0xFF;

    return count;
}



uint8_t Detect_Frame(uint8_t *datain, frame_Message_t *dataout) {
    uint8_t i = 0;
    dataout->StartFrame = ((uint16_t)datain[i++] << 8) | datain[i++];
    if (dataout->StartFrame != START_BYTES) return 0;
    dataout->TypeMessage = ((uint16_t)datain[i++] << 8) | datain[i++];
    dataout->Length      = ((uint16_t)datain[i++] << 8) | datain[i++];
    uint8_t payload_len = dataout->Length - 2; // minus CRC bytes
    for (uint8_t j = 0; j < payload_len; ++j) dataout->Data[j] = datain[i++];
    dataout->CheckFrame = ((uint16_t)datain[i++] << 8) | datain[i++];
    // verify CRC
    uint8_t sensor = dataout->Data[1];
    uint16_t crc_calc = Check_Sum(datain, i-2, sensor);
    return (crc_calc == dataout->CheckFrame);
}

float Get_Data_Sensor_LUX(frame_Message_t datain) {
    return Convert_Bytes_To_Float(
        datain.Data[2], datain.Data[3], 
        datain.Data[4], datain.Data[5]
    );
}

void Get_Data_Sensor_DHT11(frame_Message_t datain, uint8_t *temperature, uint8_t *humidity) {
    *temperature = datain.Data[2];
    *humidity    = datain.Data[3];
}

uint16_t Check_Sum_LUX(uint8_t *buf, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; ++i) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; ++j)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0);
    }
    return crc;
}

uint16_t Check_Sum_DHT11(uint8_t *buf, uint8_t len) {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < len; ++i) sum += buf[i];
    return sum;
}

uint16_t Check_Sum(uint8_t *buf, uint8_t len, uint8_t sensor_type) {
    if (sensor_type == SENSOR_LUX)    return Check_Sum_LUX(buf, len);
    if (sensor_type == SENSOR_DHT11)  return Check_Sum_DHT11(buf, len);
    return 0;
}
