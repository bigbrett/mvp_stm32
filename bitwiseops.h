#pragma once

#define BIT7MASK 0b10000000
#define BIT6MASK 0b01000000
#define BIT5MASK 0b00100000
#define BIT4MASK 0b00010000
#define BIT3MASK 0b00001000
#define BIT2MASK 0b00000100
#define BIT1MASK 0b00000010
#define BIT0MASK 0b00000001
#define BIT7(b) ((b & BIT7MASK) >> 7)
#define BIT6(b) ((b & BIT6MASK) >> 6)
#define TOP2(b) ((b & (BIT7MASK | BIT6MASK)) >> 6)
#define BOTTOM6(b) (b & 0b00111111) // mask off top two bits (byte & b00111111)
