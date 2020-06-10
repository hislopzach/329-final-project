#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#include "keypad.h"
#include "lcd.h"
#include "string.h"
#include "timer.h"

#define INPUT_DELAY 100
#define MESSAGE_DURATION 500
#define NUM_CUPS 6

// are these needed?
// we just need to set up interrupts for each one
#define CUP0_PORT P2
#define CUP0_PIN BIT3
#define CUP1_PORT P2
#define CUP1_PIN BIT4
#define CUP2_PORT P2
#define CUP2_PIN BIT5
#define CUP3_PORT P2
#define CUP3_PIN BIT6
#define CUP4_PORT P2
#define CUP4_PIN BIT7
#define CUP5_PORT P3
#define CUP5_PIN BIT0

#define RESTART_PORT P1
#define RESTART_PIN BIT0

// times
#define GOD_SEC 15
#define EXPERT_SEC 30
#define HARD_SEC 60
#define MED_SEC 90
#define EASY_SEC 120
#define BABY_SEC 180
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

  // char top_line[LCD_LINESIZE], bottom_line[LCD_LINESIZE];

  char key_pressed;
  uint32_t timer_seconds;
  int digit_pressed;
  int digit, is_winner = 0;

  // temp:
  uint32_t timer_value = 0;

  // state machine:
  // for every loop, perform the actions for the present state
  // then set set present state to the next state and repeat
  while (1)
  {
    // perform actions for current state
    switch (present_state)
    {
      case INIT:
        MAP_Interrupt_enableMaster();
        LCD_init();
        keypad_init();
        timer_init();
        init_cup_ports();
        // set restart port as output
        RESTART_PORT->DIR &= RESTART_PIN;
        next_state = RESET;

      case RESET:
        LCD_clear();
        zero_array(cup_array, NUM_CUPS);
        // reset_string(top_line, LCD_LINESIZE);
        // reset_string(bottom_line, LCD_LINESIZE);
        is_winner = 0;
        // set reset pin low
        RESTART_PORT->OUT &= ~RESTART_PIN;
        next_state = READY;
        break;
      case READY:
        write_keypad_prompt();
        digit_pressed = keypad_getint_blocking();
        // if invalid key pressed do it again
        while (digit_pressed > 6 && digit_pressed < 1)
        {
          // lcd key is invalid
          write_keypad_error();
          delayMs(MESSAGE_DURATION);
          write_keypad_prompt();
          // lcd prompt again
          digit_pressed = keypad_getint_blocking();
        }
        timer_seconds = get_timer_seconds(digit_pressed);
        next_state = START_GAME;
        break;
      case START_GAME:
        // enable timer interupt
        start_timer(timer_seconds);
        // go to game state
        next_state = GAME_PLAY;
        break;
      case GAME_PLAY:
        // printf("value: %u\n", MAP_Timer32_getValue(TIMER_MODULE));
        if (timer_value != get_current_time())
        {
          printf("Timer: %d\n", get_current_time());
          timer_value = get_current_time();
        }
        if (timer_is_up())
        {
          printf("Timer is over!\n");
          next_state = END_GAME;
          break;
        }
        if (all_cups_sunk(cup_array))
        {
          is_winner = 1;
          next_state = END_GAME;
          break;
        }
        next_state = UPDATE_BOARD;
        break;
      case UPDATE_BOARD:
        write_game_state(cup_array, timer_value);
        next_state = GAME_PLAY;
        break;
      case END_GAME:
        // check score
        write_game_over(is_winner, timer_value);
        // wait until any key pressed
        keypad_getkey_blocking();
        // disable interrupts
        stop_timer();

        // set restart pin high to rpi
        RESTART_PORT->OUT &= RESTART_PIN;
        // wait for any key, then go to reset
        next_state = RESET;
        break;
    }

    // load next state
    previous_state = present_state;
    present_state = next_state;
  }
}

void write_game_over(int winner, int timer)
{
  char top_line[LCD_LINESIZE], bottom_line[LCD_LINESIZE];
  if (winner)
  {
    sprintf(top_line, "You Won !!!");
    sprintf(bottom_line, "Time taken: %d", timer);
  }
  else
  {
    sprintf(top_line, "You Lost :(");
    sprintf(bottom_line, "Out of time!");
  }
  LCD_write_strings(top_line, bottom_line);
}

void write_keypad_prompt(void)
{
  char top_line[LCD_LINESIZE], bottom_line[LCD_LINESIZE];
  // LCD_clear();
  strcpy(top_line, "Enter Difficulty: ");
  strcpy(bottom_line, "1 Easy - 6 Hard");
  LCD_write_strings(top_line, bottom_line);
}

void write_keypad_error(void)
{
  char top_line[LCD_LINESIZE];
  strcpy(top_line, "Invalid setting");
  LCD_write_strings(top_line, NULL);
}

void write_game_state(int* cup_array, uint32_t timer)
{
  char top_line[LCD_LINESIZE], bottom_line[LCD_LINESIZE];
  char reprs[NUM_CUPS];
  char current_char;
  int i;
  for (i = 0; i < NUM_CUPS; i++)
  {
    reprs[i] = get_repr(cup_array[i], i);
  }
  sprintf(top_line, "%c %c %c       Time", reprs[3], reprs[4], reprs[5]);
  sprintf(bottom_line, " %c%c%c         %d", reprs[1], reprs[0], reprs[2],
          timer);
  LCD_write_strings(top_line, bottom_line);
}

char get_repr(int value, int index)
{
  if (index == 0)
  {
    return value ? 'X' : 'O';
  }
  else
  {
    return value ? 'x' : 'o';
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

int all_cups_sunk(int* cups)
{
  int i;
  for (i = 0; i < NUM_CUPS; i++)
  {
    if (cups[i] == 0)
    {
      return 0;
    }
  }
  return 1;
}

uint32_t get_timer_seconds(uint8_t digit)
{
  switch (digit)
  {
    case 1:
      return BABY_SEC;
      break;
    case 2:
      return EASY_SEC;
      break;
    case 3:
      return MED_SEC;
      break;
    case 4:
      return HARD_SEC;
      break;
    case 5:
      return EXPERT_SEC;
      break;
    case 6:
      return GOD_SEC;
      break;
  }
}

void init_cup_ports(void)
{
  // set cup pins as inputs
  CUP0_PORT->DIR = ~(CUP0_PIN | CUP1_PIN | CUP2_PIN | CUP3_PIN | CUP4_PIN);
  CUP5_PORT->DIR = ~CUP5_PIN;

  // enable interrupts for ports
  CUP0_PORT->IE = (CUP0_PIN | CUP1_PIN | CUP2_PIN | CUP3_PIN | CUP4_PIN);
  CUP5_PORT->IE = CUP5_PIN;
  NVIC_EnableIRQ(PORT2_IRQn);
  NVIC_EnableIRQ(PORT3_IRQn);
}

void PORT2_IRQHandler(void)
{
  int cup_num;
  // check interrupt flag to see which port interrupted
  cup_array[cup_num] = 1;
  // clear interrupt flag
}
// port x irq(void) {
//	clear interrupt flag
//  cup_array[cup_num] = 1;
