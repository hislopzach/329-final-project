#include "timer.h"

static volatile unsigned int seconds_elapsed = 0;
static volatile unsigned int timer_limit = 0;
void timer_init(void)
{
  Timer32_initModule(TIMER_MODULE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                     TIMER32_PERIODIC_MODE);
  MAP_Interrupt_enableInterrupt(INT_T32_INT1);
}

int timer_is_up(void)
{
  return seconds_elapsed >= timer_limit;
}

unsigned int get_current_time(void)
{
  return timer_limit - seconds_elapsed;
}

void start_timer(uint32_t seconds)
{
  timer_limit = seconds;
  seconds_elapsed = 0;
  // timer will interrupt every second
  Timer32_setCount(TIMER_MODULE, ONE_SEC);
  Timer32_enableInterrupt(TIMER_MODULE);
  Timer32_startTimer(TIMER_MODULE, false);
}

void stop_timer(void)
{
  seconds_elapsed = 0;
  Timer32_disableInterrupt(TIMER_MODULE);
  Timer32_haltTimer(TIMER_MODULE);
}

void T32_INT1_IRQHandler(void)
{
  Timer32_clearInterruptFlag(TIMER_MODULE);
  seconds_elapsed++;
}
