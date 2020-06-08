#include "timer.h"

static volatile unsigned int seconds_elapsed = 0;
static volatile unsigned int timer_limit = 0;
void timer_init(void)
{
  MAP_Timer32_initModule(TIMER_MODULE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                         TIMER32_PERIODIC_MODE);
  MAP_Timer32_registerInterrupt(TIMER32_0_INTERRUPT, timer_32_interrupt);
}

int timer_is_up(void)
{
  return seconds_elapsed >= timer_limit;
}

unsigned int get_current_time(void)
{
  return timer_limit - seconds_elapsed;
}

void start_timer(uint32_t seconds, uint32_t count)
{
  timer_limit = seconds;
  seconds_elapsed = 0;
  MAP_Timer32_setCount(TIMER_MODULE, count);
  MAP_Timer32_enableInterrupt(TIMER_MODULE);
  MAP_Timer32_startTimer(TIMER_MODULE, false);
}

void stop_timer(void)
{
  MAP_Timer32_disableInterrupt(TIMER_MODULE);
  MAP_Timer32_haltTimer(TIMER_MODULE);
}

void timer_32_interrupt(void)
{
  Timer32_clearInterruptFlag(TIMER_MODULE);
  seconds_elapsed++;
}