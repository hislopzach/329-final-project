#include "proximity.h"
static int TransmitFlag;

// we can use the same scl for every sensor to save space

// msp sends command to one sensor, slave_addr
// gets response
// how to map from "slave addr" to sensor index?
//    function that uses a switch to retur the index
// reading sequentially will be easier to associate cup with result
//    could we miss the trigger though?

// alternative: use a different serial bus for each sensor?
// then we could have separate interupts
// but we would be limited in th enumber of sensors (4? or 2?)

void InitSensor(uint8_t device_addr,
                DIO_PORT_Interruptable_Type* port,
                uint16_t sda_pin,
                uint16_t scl_pin)
{
  port->SEL0 |= sda_pin | scl_pin;  // Set I2C pins of eUSCI_B0

  // Enable eUSCIB0 interrupt in NVIC module
  NVIC->ISER[0] = 1 << ((EUSCIB0_IRQn)&31);

  // Configure USCI_B0 for I2C mode
  EUSCI_B0->CTLW0 |= EUSCI_A_CTLW0_SWRST;       // Software reset enabled
  EUSCI_B0->CTLW0 = EUSCI_A_CTLW0_SWRST |       // Remain eUSCI in reset mode
                    EUSCI_B_CTLW0_MODE_3 |      // I2C mode
                    EUSCI_B_CTLW0_MST |         // Master mode
                    EUSCI_B_CTLW0_SYNC |        // Sync mode
                    EUSCI_B_CTLW0_SSEL__SMCLK;  // SMCLK

  EUSCI_B0->BRW = 30;                       // baudrate = SMCLK / 30 = 100kHz
  EUSCI_B0->I2CSA = device_addr;            // Slave address
  EUSCI_B0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;  // Release eUSCI from reset

  EUSCI_B0->IE |= EUSCI_A_IE_RXIE |  // Enable receive interrupt
                  EUSCI_A_IE_TXIE;
}

void setI2CAddr(uint8_t addr)
{
  EUSCI_B0->CTLW0 |= EUSCI_A_CTLW0_SWRST;  // enter reset to allow change
  EUSCI_B0->I2CSA = addr;
  EUSCI_B0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;  // Release eUSCI from reset
}

void WriteCommand(uint8_t cmd_addr, uint8_t data_byte)
{
  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TR;     // Set transmit mode (write)
  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;  // I2C start condition

  while (!TransmitFlag)
    ;  // Wait for EEPROM address to transmit
  TransmitFlag = 0;

  EUSCI_B0->TXBUF = cmd_addr;  // Send the command address

  while (!TransmitFlag)
    ;  // Wait for the transmit to complete
  TransmitFlag = 0;

  EUSCI_B0->TXBUF = data_byte;  // Send the byte

  while (!TransmitFlag)
    ;  // Wait for the transmit to complete
  TransmitFlag = 0;

  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTP;  // I2C stop condition
}

uint8_t ReadSensor(uint8_t reg_addr)
{
  uint8_t result = 0;

  // write address to be read
  WriteCommand(SENSOR_CMD, reg_addr);

  EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_TR;    // Set receive mode (read)
  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;  // I2C start condition (restart)

  // Wait for start to be transmitted
  while ((EUSCI_B0->CTLW0 & EUSCI_B_CTLW0_TXSTT))
    ;

  // set stop bit to trigger after first byte
  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTP;

  while (!TransmitFlag)
    ;  // Wait to receive a byte
  TransmitFlag = 0;

  result = EUSCI_B0->RXBUF;  // Read byte from the buffer

  return result;
}

uint16_t ReadProximity(void)
{
  uint8_t ReceiveHi, ReceiveLo;
  uint16_t Result = 0;
  ReceiveLo = ReadSensor(PROX_READ_LO);
  ReceiveHi = ReadSensor(PROX_READ_HI);
  Result = (ReceiveHi << 8) | ReceiveLo;
  return Result;
}

void EUSCIB0_IRQHandler(void)
{
  if (EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG0)  // Check if transmit complete
  {
    EUSCI_B0->IFG &= ~EUSCI_B_IFG_TXIFG0;  // Clear interrupt flag
    TransmitFlag = 1;                      // Set global flag
  }

  if (EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG0)  // Check if receive complete
  {
    EUSCI_B0->IFG &= ~EUSCI_B_IFG_RXIFG0;  // Clear interrupt flag
    TransmitFlag = 1;                      // Set global flag
  }
}

void SetProximityThresh(uint16_t threshold)
{
  uint8_t upper, lower;
  upper = threshold >> 8;
  lower = threshold & 0xFF;
  WriteCommand(HI_THRESH_HIGHER, upper);
  WriteCommand(HI_THRESH_LOWER, lower);
}