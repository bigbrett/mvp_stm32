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

	// Set PSC and ARR to achieve the desired fundemental frequency of 1kHz
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
	TIM4->TIMx_CCR1 = newval/41 + 100;
}

void pwm_ch2_updateDutyCycle(uint32_t newval)
{
	TIM4->TIMx_CCR2 = newval/41 + 100;
}

/*
 * Kill the motors
 */
void kill_pwm()
{
	// TODO
	pwm_ch2_updateDutyCycle( MOTOR_OFF );
	pwm_ch2_updateDutyCycle( MOTOR_OFF );
}

