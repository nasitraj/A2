#include <PWM.h>

void setDutyCycle(byte d0A, byte d0B, byte d2B)
{
		OCR0A = d0A;
		OCR0B = d0B;
		OCR2B = d2B;
}

void initDutyCycle(){
DDRD |= (1 << RedPin | 1<< GreenPin | 1<< BluePin);
	TCCR0A = 0b10100011; // set 10 for COM0A and B and set 011 for WGM
	TCCR2A = 0b00100011; // set 10 for COM2B and set 011 for WGM
	TCCR0B = 0b00000011; // set the prescalar
	TCCR2B = 0b00000011; // set the prescalar
	TCNT0 = 0;
	TCNT2 = 0;
}