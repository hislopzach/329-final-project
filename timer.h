#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#define TIMER_MODULE TIMER32_0_BASE
#define ONE_SEC 300000
void timer_init(void);
void set_timer(unsigned int seconds);
uint32_t get_current_time(void);
void start_timer(uint32_t seconds);
void stop_timer(void);
int timer_is_up(void);
void T32_INT1_IRQHandler(void);