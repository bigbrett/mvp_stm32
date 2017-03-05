/*
 * servo.c - Servo Motor support for stm32f4xx board
 *
 *  Created on: Feb 7, 2015
 *  For use with ENGS 62 Lab 4
 *      Author: Brett Nicholas
 */
#include "servo.h"


/* PRIVATE: Enables Port B pin 6 for Alternate Function 2 (TIM4 ch1) for servo control */
void enable_PB6_PB7_AF2()
{
	/* Turn on GPIOB's clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

	/* Clear any existing GPIOB pin 6 mode settings */
	GPIOB->MODER &= GPIOx_MODER_PIN6_MASK & GPIOx_MODER_PIN7_MASK;   //(0xffff0fff)

	// configure port B pin 6 and 7 for AF output
	GPIOB->MODER |= GPIOx_MODER_PIN6_AF | GPIOx_MODER_PIN7_AF; // configure Port B pin 6 for AF
//	GPIOB->MODER |= GPIOx_MODER_PIN7_AF; // configure Port B pin 7 for AF

	// Clear pin 6 and 7 AF mode
	GPIOB->AFRL &= GPIOx_AFRL_AFRL6_MASK & GPIOx_AFRL_AFRL7_MASK;
//	GPIOB->AFRL &= GPIOx_AFRL_AFRL7_MASK;

	// Set pin 6 and 7 AF mode to AF2
	GPIOB->AFRL |= GPIOx_AFRL_AFRL6_AF2 | GPIOx_AFRL_AFRL7_AF2; // map TIM4 ch 1 to Pin 6 by setting AFRL as AF2
//	GPIOB->AFRL |= GPIOx_AFRL_AFRL7_AF2; // map TIM4 ch 2 to Pin 7 by setting AFRL as AF2

	// Enable tim4 peripheral clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM4;
}


/* PUBLIC: Initializes timer to interact with servo motor via PWM on tim4 signal */
void tim4_pwm_ch1_ch2_init()
{
	// Enable port B pin 6 and 7, and configure for AF2
	enable_PB6_PB7_AF2();

	// configure CCMR1 OC1M and OC2M field as "PWM mode 1" (bits 6:4 and bits 14:11 <= 110)
	TIM4->TIMx_CCMR1 |= TIMx_CCMR1_CH1_PWMMODE1EN | TIMx_CCMR1_CH2_PWMMODE1EN;

	// capture compare on ch2 and ch2 output enable
	TIM4->TIMx_CCER |= 0x11; // bit 4 and bit 0 high

	// Set PSC and ARR to achieve a pwm frequency of 50Hz from the 16MHz system clock
	// ((16*10^6)/160)/2000 = 50
	TIM4->TIMx_PSC = 159; // SYSCLOCK/(PSC+1) = CK_CNT; and SYSCLOCK = 16Mhz
	TIM4->TIMx_ARR = 2000; // 1/20kHz = 50 Hz

	// Set CCR1 to achieve desired 10% duty cycle
	TIM4->TIMx_CCR1 = 150;
	TIM4->TIMx_CCR2 = 150;

	// Enable timers by writing 1 to CEN field in TIM4_CR1
	TIM4->TIMx_CR1 |= 0x1;
}


/* updates TIM4 duty cycle to value passed as argument */
void pwm_ch1_updateDutyCycle(uint32_t newval)
{
	//TIM4->TIMx_CCR1 = newval/41 + 100;
	TIM4->TIMx_CCR1 = newval/10;
}

void pwm_ch2_updateDutyCycle(uint32_t newval)
{
	//TIM4->TIMx_CCR2 = newval/41 + 100;
	TIM4->TIMx_CCR2 = newval/10;
}


/*
 * R MOTOR Corresponds to ch1
 * L MOTOR corresponds to ch2
 */
void pwm_updateDutyCycles(Motor_t mot, uint32_t val)
{
	if (mot == RMOTOR)
	{
		TIM4->TIMx_CCR1 = val;
		return;
	}

	if (mot == LMOTOR)
	{
		TIM4->TIMx_CCR2 = val;
		return;
	}
}


/*
 * Kill the motors
 */
void kill_pwm()
{
	// TODO
	pwm_ch1_updateDutyCycle( MOTOR_OFF );
	pwm_ch2_updateDutyCycle( MOTOR_OFF );
}


/*
 * Scale speed commands into PWM range and update
 * PWM output on TIM4
 *
 * XBee speed commands are to be sent in contiguous streams of 4 USART
 * words, that honor the following byte order:
 * 		first word  (XBee_RX_buf[0]): top 6 bits (most significant 6) of R Motor speed
 * 		second word (XBee_RX_buf[1]): bottom 6 bits (least significant 6) of R Motor speed
 * 		third word  (XBee_RX_buf[2]): top 6 bits (most significant 6) of L Motor speed
 * 		fourth word (XBee_RX_buf[3]): bottom 6 bits (least significant 6) of L Motor speed
 */
void updateMotorSpeed(uint8_t *cmd_buf)
{
	/* Extract speed commands for each motor */
//	uint16_t top    = BOTTOM6(cmd_buf[0]);
//	top = top << 6;
//	uint16_t bottom = BOTTOM6(cmd_buf[1]);
//	uint32_t lspeed = top | bottom;
//	uint32_t rspeed = (BOTTOM6(cmd_buf[2]) << 6) | BOTTOM6(cmd_buf[3]);
	uint32_t lspeed = (BOTTOM6(cmd_buf[0]) << 6) | BOTTOM6(cmd_buf[1]);
	uint32_t rspeed = (BOTTOM6(cmd_buf[2]) << 6) | BOTTOM6(cmd_buf[3]);

	/* Scale speed commands into the PWM range */
	lspeed = MIN_DUTY_CYCLE + PWM_MAP_SLOPE * lspeed;
	rspeed = MIN_DUTY_CYCLE + PWM_MAP_SLOPE * rspeed;

	// TODO check bounds on speed commands

	/* Update PWM duty cycles
	 * R Motor = ch1
	 * L Motor = ch2 */
	pwm_ch1_updateDutyCycle(rspeed);
	pwm_ch2_updateDutyCycle(lspeed);
}
