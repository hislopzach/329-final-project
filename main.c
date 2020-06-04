#include "lcd.h"
#include "string.h"

#define INPUT_DELAY 100
#define MESSAGE_DURATION 500
#define TURN_DURATION 8000
#define NUM_ROWS 4
#define NUM_COLS 4
#define BOARD_SIZE (NUM_COLS * NUM_ROWS)

/**
 * CPE 329 Final Project - Connect 4 Cup Pong
 */

/* enum describing all possible states for the state machine */
typedef enum state
{
  INIT,
  RESET,
  GAME_START,
  GAME_OVER,
  NEXT_TURN,     // switch current team,
  CHECK_BOARD,   // check current board to see if anyone has won
  UPDATE_BOARD,  // write current state of board to LEDs
} state;

// start => turn => check_board => game_over
//                             |=> next turn

void reset_array(char* arr, size_t length);
void zero_array(int* arr, size_t length);
void board_set(int row, int col, int value);
int board_get(int row, int col);
int place_chip(int col, int color);
// globals
int board[BOARD_SIZE];    // keeps track of current board statr
int col_count[NUM_COLS];  // keeps track of number of tokens in each column

void main(void)
{
  state present_state = INIT;
  state next_state;
  state previous_state;

  // color stores the color value for each team
  int color[2] = {1, 2};

  // char top_line[LCD_LINESIZE], bottom_line[LCD_LINESIZE];

  char last_pressed;
  int digit;
  int check_key = 1;

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
        // sensor_init();
        next_state = RESET;

      case RESET:
        printf("Reset state\n");
        LCD_clear();
        zero_array(board, BOARD_SIZE);
        // reset_array(top_line, LCD_LINESIZE);
        // reset_array(bottom_line, LCD_LINESIZE);
        // go to locked state
        next_state = LOCKED;
        break;
      case INCORRECT_KEY:
        printf("Incorrect key state\n");
        LCD_clear();
        strcpy(top_line, "INCORRECT KEY");
        LCD_write_strings(top_line, NULL);
        delayMs(MESSAGE_DURATION);
        next_state = RESET;
        break;
    }

    // load next state
    previous_state = present_state;
    present_state = next_state;
  }
}

/* Fills a char array with null characters */
void reset_array(char* arr, size_t length)
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
