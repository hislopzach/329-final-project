void reset_string(char* arr, size_t length);
void zero_array(int* arr, size_t length);
int all_cups_sunk(int* cups);
uint32_t get_timer_seconds(uint8_t digit);
char get_repr(int value, int index);
void write_game_state(int* cup_array, uint32_t timer);
void write_keypad_error(void);
void write_keypad_prompt(void);
void write_game_over(int winner, int timer);