#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "keypad.h"
#include "lcd.h"
#include "string.h"

#define INPUT_DELAY 100
#define MESSAGE_DURATION 500
#define NUM_CUPS 6

// are these needed?
// we just need to set up interrupts for each one
#define CUP0_PORT P1
#define CUP0_PIN BIT0
#define CUP1_PORT P1
#define CUP1_PIN BIT0
#define CUP2_PORT P1
#define CUP2_PIN BIT0
#define CUP3_PORT P1
#define CUP3_PIN BIT0
#define CUP4_PORT P1
#define CUP4_PIN BIT0
#define CUP5_PORT P1
#define CUP5_PIN BIT0

// times (assuming 3Mhz clock)
#define ONE_SEC 3000000
#define EXPERT_SEC 30
#define HARD_SEC 60
#define MED_SEC 90
#define EASY_SEC 120
#define EXPERT_COUNT EXPERT_SEC* ONE_SEC
#define HARD_COUNT HARD_SEC* ONE_SEC
#define MED_COUNT MED_SEC* ONE_SEC
#define EASY_COUNT EASY_SEC* ONE_SEC

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
  GAME_PLAY,
  UPDATE_BOARD,  // write current state of board to LEDs
} state;

// start => turn => check_board => game_over
//                             |=> next turn

// globals
int cup_array[NUM_CUPS];

void* cup_ports[] = {CUP0_PORT, CUP1_PORT, CUP2_PORT,
                     CUP3_PORT, CUP4_PORT, CUP5_PORT};
uint16_t cup_pins[] = {CUP0_PIN, CUP1_PIN, CUP2_PIN,
                       CUP3_PIN, CUP4_PIN, CUP5_PIN};

void main(void)
{
  state present_state = INIT;
  state next_state;
  state previous_state;

  char top_line[LCD_LINESIZE], bottom_line[LCD_LINESIZE];

  char key_pressed;
  uint32_t timer_count, timer_seconds;
  int digit_pressed;
  int digit, is_winner = 0;

  // temp:
  uint32_t timer_value;

  // setup timer A to interrupt every (2) second(s) and decrement timer
  // in the main loop update the display to show the current timer and cup
  // status

  // set up interrutps for each pin that represents cup state

  // state machine:
  // for every loop, perform the actions for the present state
  // then set set present state to the next state and repeat
  while (1)
  {
    // perform actions for current state
    switch (present_state)
    {
      case INIT:
        // LCD_init();
        // keypad_init();
        timer_init();
        next_state = RESET;

      case RESET:
        // LCD_clear();
        // zero_array(cup_array, NUM_CUPS);
        // reset_string(top_line, LCD_LINESIZE);
        // reset_string(bottom_line, LCD_LINESIZE);
        // is_winner = 0;
        // digit_pressed = 0;
        timer_seconds = HARD_SEC;
        timer_count = HARD_COUNT;
        next_state = GAME_PLAY;
        // next_state = READY;
        break;
      case READY:
        // prompt for difficulty level
        digit_pressed = keypad_getint_blocking();
        // if invalid key pressed do it again
        while (digit_pressed > 9)
        {
          // lcd invalid key
          // lcd prompt again
          digit_pressed = keypad_getint_blocking();
        }
        // timer_count = get_timer_settings(digit_pressed);
        timer_seconds = HARD_SEC;
        timer_count = HARD_COUNT;

        break;
      case START_GAME:
        // enable timer interupt
        start_timer(timer_seconds, timer_count);
        // go to game state
        next_state = GAME_PLAY;
        break;
      case GAME_PLAY:
        if (timer_value != get_current_time())
        {
          printf("Timer: %d\n", get_current_time());
          timer_value = get_current_time();
        }
        if (timer_is_up())
        {
          printf("Timer is over!\n");
        }
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
