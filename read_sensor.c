#include "proximity.h";

// addresses
#define SENSOR_ADDR 0x13

// values
#define PROX_ENABLE BIT1

void main(void)
{
  uint16_t value;
  int i;
  WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;  // Stop watchdog timer
  __enable_irq();                              // Enable global interrupt

  InitSensor(SENSOR_ADDR, P1, BIT6, BIT7);

  // enable proximity continuous measurement
  WriteCommand(SENSOR_CMD, PROX_ENABLE);

  // read prox measurments forever
  while (1)
  {
    value = ReadProximity();
    printf("proximity: %d\n", value);
  }
}

// make irqs for each pin that comes from the sensor