#include <stdbool.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "msp432p401r.h"
// addresses
#define SENSOR_CMD 0x80
#define PROX_READ_HI 0x87
#define PROX_READ_LO 0x88
#define HI_THRESH_LOWER 0x8D
#define HI_THRESH_HIGHER 0x8c

// void InitSensor(uint8_t device_addr,
//                 DIO_PORT_Interruptable_Type* port,
//                 uint16_t sda_pin,
//                 uint16_t scl_pin);

// uint8_t ReadSensor(uint8_t MemAddress);
// uint16_t ReadProximity(void);
// void WriteCommand(uint8_t MemAddress, uint8_t MemByte);
// void EUSCIB0_IRQHandler(void);
// void SetProximityThresh(uint16_t threshold);
