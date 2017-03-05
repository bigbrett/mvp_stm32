/*
 * servo.h - Servo Motor support for stm32f4xx board
 *
 *  Created on: Feb 7, 2015
 *  For use with ENGS 62 Lab 4
 *      Author: Brett Nicholas
 */
#include "stdint.h"
#include "timer.h"
#include "stm32f4xx_timer.h"
#include "stm32f4xx_gpio.h"

typedef enum {RMOTOR, LMOTOR} Motor_t;

// TODO duty cycle ranges
#define MOTOR_OFF 1500
#define MAX_DUTY_CYCLE 1000
#define MIN_DUTY_CYCLE 2000


/* Controls position of servo motor based on value of external potentiometer */
void tim4_pwm_ch1_ch2_init();
void pwm_ch2_updateDutyCycle(uint32_t);
void pwm_ch2_updateDutyCycle(uint32_t);
void pwm_updateDutyCycle(Motor_t motor, uint32_t duty_cycle);
void kill_pwm();




