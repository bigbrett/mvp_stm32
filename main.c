#include "fsm.h"
#include "USART2.h"
#include "userbutton.h"
#include "timer.h"
#include "servo.h"


uint32_t usart2_interrupt_cnt = 0;
uint32_t usart2_interrupt_cnt_last = 0;

void USART2_callback_fn(uint8_t byte)
{
	usart2_interrupt_cnt++;
}


void userbutton_callback_fn(void)
{

}


/* Callback function for TIM2 interrupts */
void TIM7_callback_fn(void)
{
	if (usart2_interrupt_cnt == usart2_interrupt_cnt_last) {
		kill_pwm();
	}
	usart2_interrupt_cnt_last = usart2_interrupt_cnt;
}


void main(void)
{
	/* Initialize modules */
	USART2_init(USART2_callback_fn);
	LED_init();
	userbutton_init(userbutton_callback_fn);
	TIM7_init(TIM7_callback_fn);
	tim4_pwm_ch1_ch2_init();

	/* Enable interrupts - do this after initializing the system */
	__asm ("  cpsie i \n" );

	/* Wait here forever - everything is now interrupt driven */
	while(1)
	{
		;;;
	}
}
