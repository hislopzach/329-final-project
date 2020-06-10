#include "proximity.h";

// addresses
#define SENSOR_ADDR 0x13

// values
#define PROX_ENABLE BIT1

static uint8_t TXData = 0;
static uint8_t RXData = 0;
static uint8_t TXByteCtr;

// i2c config
const eUSCI_I2C_MasterConfig i2cConfig = {
    EUSCI_B_I2C_CLOCKSOURCE_SMCLK,      // SMCLK Clock Source
    3000000,                            // SMCLK = 3MHz
    EUSCI_B_I2C_SET_DATA_RATE_100KBPS,  // Desired I2C Clock of 400khz
    0,                                  // No byte counter threshold
    EUSCI_B_I2C_NO_AUTO_STOP            // No Autostop
};

// helper funcs
uint16_t merge_bytes(uint8_t upper, uint8_t lower);
uint8_t read_byte(uint8_t tx_data);

void main(void)
{
  uint16_t value;
  uint8_t upper, lower;
  int i;
  MAP_WDT_A_holdTimer();  // Stop watchdog timer

  /* Select Port 1 for I2C - Set Pin 6, 7 to input Primary Module Function,
   *   (UCB0SIMO/UCB0SDA, UCB0SOMI/UCB0SCL).
   */
  MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
      GPIO_PORT_P1, GPIO_PIN6 + GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);

  // apply config
  MAP_I2C_initMaster(EUSCI_B0_BASE, &i2cConfig);

  MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, SENSOR_ADDR);

  MAP_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

  MAP_I2C_enableModule(EUSCI_B0_BASE);

  I2C_masterReceiveSingleByte(EUSCI_B0_BASE);

  /* Enable I2C Module to start operations */
  MAP_I2C_enableModule(EUSCI_B0_BASE);
  MAP_Interrupt_enableInterrupt(INT_EUSCIB0);

  // enable RX interrupts
  MAP_I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

  // set mode to periodic proximity measurements
  MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, SENSOR_CMD);
  MAP_I2C_masterSendMultiByteFinish(EUSCI_B0_BASE, PROX_ENABLE);

  while (1)
  {
    RXData = 0;

    upper = read_byte(PROX_READ_HI);
    lower = read_byte(PROX_READ_LO);
    value = merge_bytes(upper, lower);
    printf("Prox value: %d\n");
    __delay_cycles(1500000);  // ~500ms pause between transmissions
  }
}

uint16_t merge_bytes(uint8_t upper, uint8_t lower)
{
  uint16_t result = upper << 8 | lower & 0xFF;
  return result;
}

// from eUSCI_B0
uint8_t read_byte(uint8_t tx_data)
{
  /* Making sure the last transaction has been completely sent out */
  while (MAP_I2C_masterIsStopSent(EUSCI_B0_BASE))
    ;

  /* Send addr of data to read */
  MAP_I2C_masterSendSingleByte(EUSCI_B0_BASE, tx_data);
  /*---------------------------------------------*/
  /* Now we need to initiate the read */
  /* Wait until Byte has been output to shift register */
  while (!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG0))
    ;

  // Send the restart condition, read one byte, send the stop condition right
  // away
  EUSCI_B0->CTLW0 &= ~(EUSCI_B_CTLW0_TR);
  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;
  while (MAP_I2C_masterIsStartSent(EUSCI_B0_BASE))
    ;
  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTP;

  //---------------------------------
  MAP_PCM_gotoLPM0InterruptSafe();

  return RXData;
}

void EUSCIB0_IRQHandler(void)
{
  uint_fast16_t status;

  status = MAP_I2C_getEnabledInterruptStatus(EUSCI_B0_BASE);
  MAP_I2C_clearInterruptFlag(EUSCI_B0_BASE, status);

  /* Receives bytes into the receive buffer. If we have received all bytes,
   * send a STOP condition */
  if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0)
  {
    // One-byte Read
    RXData = MAP_I2C_masterReceiveSingle(EUSCI_B0_BASE);
    MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN1);
  }
}
