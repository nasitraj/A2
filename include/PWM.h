#pragma once
#include <Arduino.h>
#define RedPin PD6
#define GreenPin PD3
#define BluePin PD5

void initDutyCycle();
void setDutyCycle(byte d0A, byte d0B, byte d2B);
		
		
	