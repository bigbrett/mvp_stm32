/*
 *
 * This is a basic implementation of XBee motor control
 * for the miniVP. It Receives a stream of serial bytes
 * from the XBee's USART, which contains speed commands
 * for both motors. These two speed commands are then
 * translated to PWM duty cycles for the motor.
 *
 * Written by Brett Nicholas
 */
#include "fsm.h"
#include "USART2.h"
#include "userbutton.h"
#include "timer.h"
#include "servo.h"
#include "bitwiseops.h"



/*
 * Globals
 */
uint32_t usart2_interrupt_cnt = 0;
uint32_t usart2_interrupt_cnt_last = 0;
uint8_t XBee_RX_buf[4] = {0,0,0,0};
uint16_t l_motor_speed = 0;
uint16_t r_motor_speed = 0;
int byte_cnt = 0;
int idx_hist = 0;

int btn_on = 0;
int kill_motors = 0;


/*
 * Function prototypes
 */

/*
 * This is the callback function for USART2, which we are using
 * to receive speed commands fromt the XBee module on board the
 * MVP. This function is responsible for parsing out the motor
 * speed values for BOTH motors from the 4 bytes received over
 * UART. Once these values are obtained, then a separate
 * function is invoked that scales and translates the speed
 * commands into PWM duty cycles, and updates the PWM generator
 *
 * It is possible for packets to be dropped or arrive out of order
 * from the Xbee, and so there are a number of ways to deal with this.
 * For now, we are stopping the motors if motor commands aren't received
 * in order, but if this happends too frequently, TODO: we should
 * actually reassemble the packets in order and try to save the command.
 *
 * XBee speed commands are to be sent in contiguous streams of 4 USART
 * words, that honor the following byte order:
 * 		first word  (byte 0): top 6 bits of R Motor speed
 * 		second word (byte 1): bottom 6 bits of R Motor speed
 * 		third word  (byte 2): top 6 bits of L Motor speed
 * 		fourth word (byte 3): bottom 6 bits of L Motor speed
 *
 */
void USART2_callback_fn(uint8_t byte)
{
	usart2_interrupt_cnt++;

	if (btn_on==1)
		return;

	// Store the data at the correct index of our speed command buffer
	uint8_t idx = TOP2(byte);
	uint8_t data = BOTTOM6(byte);
	XBee_RX_buf[idx] = BOTTOM6(data);

	// Hacky way to grab the index history
	idx_hist = (idx_hist << 2) | idx;

	/* If this byte claims to be the last command byte... */
	if (idx == 0b11)
	{
		/* We didn't sequentially get all 4 commands in order, so
		 * something went wrong and we need to stop the motors */
		if (idx_hist != 0b00011011)
		{
			XBee_RX_buf[0] = 0;
			XBee_RX_buf[1] = 0;
			XBee_RX_buf[2] = 0;
			XBee_RX_buf[3] = 0;
			kill_pwm();
			byte_cnt = 0;
		}
		else
		{ /* all looks good, and lets send the buffer */
			updateMotorSpeed(XBee_RX_buf);
			byte_cnt = 0;
		}
	}
	else
	{
		byte_cnt++;
	}

	/* If we haven't gotten bit 3 in the last 4 bytes,
	 * something is wrong and we need to stop the motors */
	if (byte_cnt > 4)
	{
	    XBee_RX_buf[0] = 0;
	    XBee_RX_buf[1] = 0;
	    XBee_RX_buf[2] = 0;
	    XBee_RX_buf[3] = 0;
		kill_pwm();
		byte_cnt = 0;
	}
}



void userbutton_callback_fn(void)
{
	/* Last btn state was off, so this should turn system to
	 * ON state. Turn on green LED and allow motors to be
	 * controlled by USART interrupts. TODO This should be relatively
	 * safe, but if issues arise we probably need to put in
	 * a mutex on btn_on
	 */
	if (btn_on==0)
	{
		btn_on = 1;
		LED_update(LED_GREEN_ON);
		LED_update(LED_RED_OFF);
	}
	else
	{	/* Otherwise we came from the ON state so
		 * we want to kill the motors, reset the
		 * cmd buffer, and ensure the USART handler cannot
		 * set the motors */
		btn_on = 0;
		LED_update(LED_RED_ON);
		LED_update(LED_GREEN_OFF);
	    XBee_RX_buf[0] = 0;
	    XBee_RX_buf[1] = 0;
	    XBee_RX_buf[2] = 0;
	    XBee_RX_buf[3] = 0;
		kill_pwm();
	}
}



/* Callback function for TIM2 interrupts */
void TIM7_callback_fn(void)
{
	USART2_putstr("*\n");
	if (usart2_interrupt_cnt == usart2_interrupt_cnt_last) {
		kill_pwm();
		USART2_putstr("MOTORS KILLED\n");
	}
	usart2_interrupt_cnt_last = usart2_interrupt_cnt;
}


/*
 * test bytes
 * byte 0 = 0b0
 * byte 1 = 'A' 0b01000001
 * byte 2 = 0b10111111
 * byte 3 = 0xFF
 */
void main(void)
{
	/* Initialize modules */
	USART2_init(USART2_callback_fn);
	LED_init();
	userbutton_init(userbutton_callback_fn);

	USART2_putstr("Hello, World!\n\0");
	tim4_pwm_ch1_ch2_init();
	//TIM7_init(TIM7_callback_fn);

//	XBee_RX_buf[0] = 0xb00000001;
//	XBee_RX_buf[1] = 0xb01000010;
//	XBee_RX_buf[2] = 0xb10000100;
//	XBee_RX_buf[3] = 0xb11001000;
//	updateMotorSpeed(XBee_RX_buf);

	/* Enable interrupts - do this after initializing the system */
	__asm ("  cpsie i \n" );

	/* Wait here forever - everything is now interrupt driven */

	// ch1 gets min val
	XBee_RX_buf[0] = 0b00000000;
	XBee_RX_buf[1] = 0b01100000;
	// ch2 gets max val
	XBee_RX_buf[2] = 0b10000001;
	XBee_RX_buf[3] = 0b11100100;
	updateMotorSpeed(XBee_RX_buf);

	while(1)
	{
		;;;
	}
}
