#include "keypad.h"
#include "lcd.h"
#include "string.h"

#define INPUT_DELAY 100
#define MESSAGE_DURATION 500
#define TURN_DURATION 8000
#define NUM_ROWS 4
#define NUM_COLS 4
#define NUM_CUPS 4
#define BOARD_SIZE (NUM_COLS * NUM_ROWS)

// times
#define THIRTY_SEC 5000

/**
 * CPE 329 Final Project - Connect 4 Cup Pong
 */

/* enum describing all possible states for the state machine */
typedef enum state
{
  INIT,
  RESET,
  READY,
  START_GAME,
  END_GAME,
  GAME_PLAY,     // switch current team,
  UPDATE_BOARD,  // write current state of board to LEDs
} state;

// start => turn => check_board => game_over
//                             |=> next turn

void reset_string(char* arr, size_t length);
void zero_array(int* arr, size_t length);
void board_set(int row, int col, int value);
int board_get(int row, int col);
int place_chip(int col, int color);
void InitSensor(uint8_t device_addr,
                DIO_PORT_Interruptable_Type* port,
                uint16_t sda_pin,
                uint16_t scl_pin);
// globals
int board[BOARD_SIZE];    // keeps track of current board statr
int col_count[NUM_COLS];  // keeps track of number of tokens in each column
int cup_array[NUM_CUPS];

void main(void)
{
  state present_state = INIT;
  state next_state;
  state previous_state;

  char top_line[LCD_LINESIZE], bottom_line[LCD_LINESIZE];

  char key_pressed;
  int digit_pressed;
  int digit, timer_count, is_winner = 0;

  // state machine:
  // for every loop, perform the actions for the present state
  // then set set present state to the next state and repeat
  while (1)
  {
    // perform actions for current state
    switch (present_state)
    {
      case INIT:
        LCD_init();
        keypad_init();
        // sensor_init(0x13, P1, BIT6, BIT7);
        // sensor_init(0x13, P1, BIT6, BIT7);
        // sensor_init(0x13, P1, BIT6, BIT7);
        // sensor_init(0x13, P1, BIT6, BIT7);
        next_state = RESET;

      case RESET:
        LCD_clear();
        zero_array(cup_array, NUM_CUPS);
        reset_string(top_line, LCD_LINESIZE);
        reset_string(bottom_line, LCD_LINESIZE);
        is_winner = 0;
        digit_pressed = 0;
        // go to locked state
        next_state = READY;
        break;
      case READY:
        // prompt for difficulty level
        // key_pressed = keypad_getkey_blocking();
        // while (key != "*" && key != "0" && key != "#")
        digit_pressed = keypad_getint_blocking();
        // if invalid key pressed do it again
        while (digit_pressed > 9)
        {
          // lcd invalid key
          // lcd prompt again
          digit_pressed = keypad_getint_blocking();
        }
        timer_count = get_timer_settings(digit_pressed);

        break;
      case START_GAME:
        // enable timer interupt
        // go to game state
        next_state = GAME_PLAY;
        break;
      case GAME_PLAY:
        // every .5 s
        // check board for win. if win => game over
        // update leds for game?
        // non blocking check for reset key
        next_state = GAME_PLAY;
        break;
      case END_GAME:
        // check score
        // print message
        // disable interrupts
        // wait for any key, then go to reset
        next_state = RESET;
        break;
    }

    // load next state
    previous_state = present_state;
    present_state = next_state;
  }
}

/* Fills a char array with null characters */
void reset_string(char* arr, size_t length)
{
  int i;
  // make buffer all null chars
  for (i = 0; i < length; i++)
  {
    arr[i] = '\0';
  }
}

/* Fills a char array with null characters */
void zero_array(int* arr, size_t length)
{
  int i;
  // make buffer all null chars
  for (i = 0; i < length; i++)
  {
    arr[i] = 0;
  }
}

void board_set(int row, int col, int value)
{
  board[col + NUM_COLS * row] = value;
}

int board_get(int row, int col)
{
  return board[col + NUM_COLS * row];
}

// returns 1 on valid, -1 on invalid
int place_chip(int col, int color)
{
  if (col_count[col] < NUM_ROWS)
  {
    board_set(col_count[col], col, color);
    col_count[col]++;
  }
  else
  {
    return -1;
  }
}

uint8_t ReadEEPROM(uint16_t MemAddress)
{
  uint8_t ReceiveByte;
  uint8_t HiAddress;
  uint8_t LoAddress;

  HiAddress = MemAddress >> 8;
  LoAddress = MemAddress & 0xFF;

  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TR;     // Set transmit mode (write)
  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;  // I2C start condition

  while (!TransmitFlag)
    ;  // Wait for EEPROM address to transmit
  TransmitFlag = 0;

  EUSCI_B0->TXBUF = HiAddress;  // Send the high byte of the memory address

  while (!TransmitFlag)
    ;  // Wait for the transmit to complete
  TransmitFlag = 0;

  EUSCI_B0->TXBUF = LoAddress;  // Send the low byte of the memory address

  while (!TransmitFlag)
    ;  // Wait for the transmit to complete
  TransmitFlag = 0;

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

  ReceiveByte = EUSCI_B0->RXBUF;  // Read byte from the buffer

  return ReceiveByte;
}
