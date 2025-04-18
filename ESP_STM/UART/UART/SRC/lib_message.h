#ifndef __LIB_MESSAGE_H
#define __LIB_MESSAGE_H

#include <stdint.h>
#include "lib_convert.h"

// Frame structure
typedef struct {
    uint16_t StartFrame;
    uint16_t TypeMessage;
    uint16_t Length;
    uint8_t  Data[12];  // sensor_type + payload bytes
    uint16_t CheckFrame;
} frame_Message_t;

#define START_BYTES                0xAA55
#define LENGTH_DEFAULT             0x0004  // port(1)+sensor(1)+CRC(2)
#define LENGTH_MESSAGE_RESPOND_LUX 0x000A  // port(1)+sensor(1)+4byte float+CRC(2)
#define LENGTH_MESSAGE_RESPOND_DHT 0x0008  // port(1)+sensor(1)+temp(1)+humi(1)+CRC(2)

#define ERROR                      0x0000

// Message types
typedef enum {
   TYPE_MESSAGE_RESPOND_DATA = 0x04,
} type_Message_e;

// Ports
typedef enum {
   PORT_A = 0x01,
   PORT_B = 0x02,
} port_e;

// Sensor types
typedef enum {
   SENSOR_LUX   = 0x01,
   SENSOR_DHT11 = 0x05,
} type_Sensor_e;

// Create response frames
uint8_t Message_Create_Frame_Respond_LUX(uint8_t port, float lux_value, uint8_t *dataout);
uint8_t Message_Create_Frame_Respond_DHT11_Full(
    uint8_t port, uint8_t temp_int, uint8_t temp_dec,
    uint8_t humi_int, uint8_t humi_dec, uint8_t *dataout);

// Parse incoming frame
uint8_t Detect_Frame(uint8_t *datain, frame_Message_t *dataout);

// Extract sensor values
float   Get_Data_Sensor_LUX(frame_Message_t dat);
void Get_Data_Sensor_DHT11(frame_Message_t dat,
    uint8_t *humi_int, uint8_t *humi_dec, uint8_t *temp_int, uint8_t *temp_dec);

// Checksum: include sensor_type
uint16_t Check_Sum(uint8_t *buf, uint8_t len, uint8_t sensor_type);

#endif // __LIB_MESSAGE_H