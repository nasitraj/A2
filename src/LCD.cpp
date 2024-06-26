#include <LCD.h>
volatile uint8_t timer_overflow_count = 0;
// initialize port pins then initialize LCD controller
void LCD_init(void)
{
	PORTS_init();

	delayMs(100); // LCD controller reset sequence
	LCD_nibble_write(0x30, 0);
	delayMs(5);
	LCD_nibble_write(0x30, 0);
	delayMs(1);
	LCD_nibble_write(0x30, 0);
	delayMs(1);

	LCD_nibble_write(0x20, 0); // use 4-bit data mode
	delayMs(1);
	LCD_command(0x28); // set 4-bit data, 2-line, 5x7 font
	LCD_command(0x06); // move cursor right
	LCD_command(0x01); // clear screen, move cursor to home
	LCD_command(0x0F); // turn on display, cursor blinking
}

// Initialize ports to output mode
void PORTS_init(void)
{
	// PB4 for LCD R/S
	// PB2 for LCD EN
	DDRB |= (1 << RS) | (1 << EN); // set pin output mode
	PORTB &= ~(1 << EN);		   // turn off EN

	// PD4-PD7 for LCD D4-D7, respectively.
	DDRC |= 0x0F; // set pin output mode
}

void LCD_nibble_write(uint8_t data, uint8_t control)
{

	// populate data bits
	PORTC = data & 0x0F; // set data bits

	// set R/S bit
	if (control & RS)
	{
		PORTB |= (1 << RS);
	}
	else
	{
		PORTB &= ~(1 << RS);
	}

	// pulse E
	PORTB |= (1 << EN);	 // pulse E high
	PORTB &= ~(1 << EN); // clear E
}

void LCD_command(uint8_t command)
{
	LCD_nibble_write(command >> 4, 0);	 // then lower nibble
	LCD_nibble_write(command & 0x0F, 0); // upper nibble first
	

	if (command < 4)
	{
		delayMs(2); // command 1 and 2 needs up to 1.64ms
	}
	else
	{
		delayMs(1); // all others 40 us
	}
}

void LCD_data(uint8_t data)
{
	LCD_nibble_write(data >> 4, RS);  
	LCD_nibble_write(data & 0x0F, RS); // upper nibble first
	 // then lower nibble

	delayMs(1);
}

//send string to LCD
void LCD_string(char *str)
{
  byte i;
  for(i=0; str[i] !='\0';i++)
  {
    LCD_data(str[i]);
  }
}

//_delay_ms( ), expects a constant, therefore,
// you can use this delay function which accepts a
// variable delay time
void delayMs(uint16_t ms)
{
	// TCNT0=0x00;
	// TCCR0B |= (1<<CS00) | (1<<CS02);
	// while (ms > 0)
	// {
	// 	while ((TIFR0 & (1<<TOV0)) == 0); //check if overflow flag is set
	// 	 TCNT0 = 0x00;
	// 	 TIFR0|=(1<<TOV0);
	// 	//_delay_ms(1);
	// 	ms--;
	// }

     // Set Timer0 prescaler to 64
    TCCR0B |= (1 << CS01) | (1 << CS00);

	unsigned long fq = 16000000UL/64;
	uint32_t tick = 1/(fq);
	//uint16_t ticks = ms/(tick*100000UL);

	
    // Calculate number of ticks for the delay
    uint16_t ticks = (ms * 16000000UL) / (64UL * 10000UL);

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
}

