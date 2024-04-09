#include <Arduino.h>
#include <avr_debugger.h>
#include <USART.h>
#include <LCD.h>
#include <PWM.h>
#define  Trigger_pin	PC5	/* Trigger pin */
#define BUTTON PC4  //interrupt source on INT0

int TimerOverflow = 0;	

volatile bool distanceFlag = true;
ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;		/* Increment Timer Overflow count */
}


ISR(PCINT1_vect)
{
	if(distanceFlag){
		distanceFlag = false;
	}else{
		distanceFlag = true;
	}
}
int main()
{

	// Add at the beginning of each project to enable debugging
	#ifdef __DEBUG__
		dbg_start();
	#endif

	char string[10];
	long count;
	double distance;
	LCD_init();
	 DDRC |= (1 << Trigger_pin);			/* Make trigger pin as output */
	 DDRB &= ~(1<<PB0);			
	 PORTB |= (1 << Trigger_pin);			/* Turn on Pull-up */
	DDRC &=~(1<<BUTTON); //set PD2/INT0 as input
  	PORTC|= (1<<BUTTON); // activate pull-up

	PCICR|= (1<<PCIE1); // set INT0 for falling edge mode
  	PCMSK1|=(1<<PCINT12); //enable PCINT12
	
	USART_init();
	initDutyCycle();
	USART_send_string("Ultrasonic");
	USART_send('\n');
	 LCD_command(1);
	 LCD_string("Ultrasonic1");
	 LCD_command(0xC0);
	
	sei();					/* Enable global interrupt */
	TIMSK1 = (1 << TOIE1);	/* Enable Timer1 overflow interrupts */
	// TCCR1A = 0;				/* Set all bit to zero Normal operation */
	
	while(1)
	{

		PORTC |= (1 << Trigger_pin);/* Give 10us trigger pulse on trig. pin to HC-SR04 */
		// Set Timer0 prescaler to 64
		TCCR0B |= (1 << CS01) | (1 << CS00);

		unsigned long fq = 16000000UL/64;
		uint32_t tick = 1/(fq);
		//uint16_t ticks = ms/(tick*100000UL);

		
		// Calculate number of ticks for the delay
		uint16_t ticks = (10 * 16000000UL) / (64UL * 1000000UL);

		// Reset Timer0 counter
		TCNT0 = 0;

		// Wait until the required number of ticks occur
		while (ticks > 0) {
			if (TIFR0 & (1 << TOV0)) {
				// Clear Timer0 overflow flag
				TIFR0 |= (1 << TOV0);
				// Decrement ticks
				ticks--;
			}
		}

		// Disable Timer0
		TCCR0B = 0;
		//_delay_us(10);
		PORTC &= (~(1 << Trigger_pin));
		
		TCNT1 = 0;			/* Clear Timer counter */
		TCCR1B = 0x41;		/* Setting to capture rising edge, No pre-scaler*/
		TIFR1 = 1<<ICF1;		/* Clear ICP flag (Input Capture flag) */
		TIFR1 = 1<<TOV1;		/* Clear Timer Overflow flag */

		/*Calculate width of Echo by Input Capture (ICP) on PortB PB0 */
		
		while ((TIFR1 & (1 << ICF1)) == 0);	/* Wait for rising edge */
		TCNT1 = 0;			/* Clear Timer counter */
		TCCR1B = 0x01;		/* Setting for capture falling edge, No pre-scaler */
		TIFR1 = 1<<ICF1;		/* Clear ICP flag (Input Capture flag) */
		TIFR1 = 1<<TOV1;		/* Clear Timer Overflow flag */
		TimerOverflow = 0;	/* Clear Timer overflow count */

		while ((TIFR1 & (1 << ICF1)) == 0); /* Wait for falling edge */
		count = ICR1 + (65535 * TimerOverflow);	/* Take value of capture register */
		/* 16MHz Timer freq, sound speed =343 m/s,  17150 x Timer value * 0.0625 x 10 ^ -6 = Timer value / 932.8*/
		distance = (double)count / 932.8;
		if(distance < 10){
				setDutyCycle(255,0, 0); 
			}else if(distance > 10){
				setDutyCycle(0,255, 0); 
			}
		if(distanceFlag){
			
			distance *= 10;
			dtostrf(distance, 2, 2, string);/* Convert distance into string */
			strcat(string, " mm   ");
		}else{
			dtostrf(distance, 2, 2, string);/* Convert distance into string */
			strcat(string, " cm   ");
		}

		
		USART_send_string("Dist = ");
		LCD_command(1);
		LCD_string("Ultrasonic");
		LCD_command(0xC0);
		LCD_string(string);
		USART_send_string(string);	/* Print distance on serial monitor */
		USART_send('\n');
		delayMs(20);
		//_delay_ms(2000);
	}



}