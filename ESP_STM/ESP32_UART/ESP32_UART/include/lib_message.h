#ifndef __LIB_MESSAGE_H
#define __LIB_MESSAGE_H

#include <stdint.h>
#include "lib_convert.h"

// Frame structure
typedef struct {
    uint16_t StartFrame;        // 0xAA55
    uint16_t TypeMessage;       // Message type: e.g. respond, ask, etc.
    uint16_t Length;            // Length of Data[] + CheckFrame
    uint8_t  Data[12];          // Data payload: sensor_type + actual data
    uint16_t CheckFrame;        // CRC or checksum
} frame_Message_t;

// --- Constants ---
#define START_BYTES                 0xAA55     // Start frame marker
#define LENGTH_DEFAULT              0x0004     // Example: port(1) + data(1) + CRC(2)
#define LENGTH_DEFAULT_CHECK_SUM    0x0002     // Length of checksum
#define LENGTH_MESSAGE_RESPOND_LUX  0x000A     // port(1) + type(1) + float(4) + CRC(2)
#define LENGTH_MESSAGE_RESPOND_DHT  0x0008     // port(1) + type(1) + temp(1) + humi(1) + CRC(2)
#define ERROR                       0x0000     // Error flag or invalid checksum

// --- Enums for message type ---
typedef enum {
    TYPE_MESSAGE_SET_PORT       = 0x01,
    TYPE_MESSAGE_CONFIRM_PORT   = 0x02,
    TYPE_MESSAGE_ASK_DATA       = 0x03,
    TYPE_MESSAGE_RESPOND_DATA   = 0x04,
    // TYPE_MESSAGE_ACK_DATA    = 0x05,    // Ignored as per your request
} type_Message_e;

// --- Enums for port ---
typedef enum {
    PORT_A = 0x01,
    PORT_B = 0x02,
} port_e;

// --- Enums for sensor types ---
typedef enum {
    SENSOR_LUX   = 0x01,
    SENSOR_DHT11 = 0x05,
    // You can add more like SENSOR_NTC = 0x03 if needed
} type_Sensor_e;

// --- Function prototypes ---
// Frame creation
uint8_t Message_Create_Frame_Respond_LUX(uint8_t port, float lux_value, uint8_t *dataout);
uint8_t Message_Create_Frame_Respond_DHT11_Full(   uint8_t port, uint8_t temp_int, uint8_t temp_dec,uint8_t humi_int, uint8_t humi_dec, uint8_t *dataout);

// Frame parsing
uint8_t Detect_Frame(uint8_t *datain, frame_Message_t *dataout);

// Extract sensor data
float  Get_Data_Sensor_LUX(frame_Message_t datain);
void   Get_Data_Sensor_DHT11(frame_Message_t datain, uint8_t *temperature, uint8_t *humidity);

// Checksum
uint16_t Check_Sum(uint8_t *buf, uint8_t len, uint8_t sensor_type);

#endif
