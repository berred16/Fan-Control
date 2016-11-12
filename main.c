/*
Copyright (C) 2016  Bernhard Redemann (bernd.red@b-redemann.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
Measures the temperatur given by an LMT87 sensor.
Controls 2 Outputs in order to switch the fan to 6V or 12V operation
ADC input: 
5...2,3V - off
2,3V...1,6V - 6V  operation  
1,6V or less - 12V operation 

Version 1.0a / 2016, Nov. 08th

Hardware setup:

Attiny13 (DIP8 package, 8 pins)

				PB5 |1------U------8| VCC 5V
				PB3 |2             7| PB2 ADC1 LMT87 <-
				PB4 |3             6| PB1 6V oper. ->
				GND |4-------------5| PB0 12V oper. ->

Fuses: 
*/

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#define F_CPU 9600000UL

void adc_init(void)
	{
		// ADC0 
		ADMUX = (0<<REFS0)|(0<<MUX1)|(1<<MUX0); // ADC1, AREF=VCC (5V)
		// single mode, adc clock = 1/128 of 8MHz = 62,5kHz, interrupt enabled 
		ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0); 
		/* Datasheet
		Bit 7: 1 ADEN Enable
		Bit 6: 0 ADSC Start Conversation
		Bit 5: 0 ADATE Trigger enable
		Bit 4: 0 ADIF Interrupt flag
		Bit 3: 1 ADIE Interrupt enable
		Bit 2: 1 PS2 Prescalar
		Bit 1: 1 PS1 Prescalar
		Bit 0: 0 PS0 Prescalar
		*/
	}

// variables for temperatur check
char i;
bool fan_6, fan_12;
uint16_t temperature;

void check_temperature(void)
{


if (temperature <= 460) // equals to 2,24V = 29°C
	{
		if (temperature >= 442) // equals to 1,90V = 35°C
			{
				fan_12 = false;
				fan_6 = true; // 6V operation between 29°C and 35°C
			}
	}

if (temperature > 460) // fans off when < 29°C
	{
		fan_6 = false; 
		fan_12 = false; 
	}

if (temperature < 442) // fans on when > 35°C
	{
		fan_6 = false; 
		fan_12 = true;	
	}

if (fan_6) // on port PB0
	{
		PORTB = 0x01;
	}

if (fan_12) // on port PB1
	{
		PORTB = 0x02;
	}
	
if (!fan_6 && !fan_12)
	{
		PORTB = 0x00;
	}
				
	// do not start ad converter every ISR
	// not necessary
	if (i > 250) // 250 just tested, is o.k.
	{
		ADCSRA |= (1<<ADSC);
		i = 0;
	}
	i++;
}

void timer_init(void)
	{
		TIMSK0 |= (1 << TOIE0); // Timer interrupts enable
		TCCR0B = (1<<CS02)|(0<<CS01)|(1<<CS00);
	}

ISR (SIG_OVERFLOW0) // Timer overflow vector
	{
		check_temperature();
	}
	
ISR (ADC_vect) // ADC interrupt vector
	{
		temperature = ADCW; // 1024 * 5;
	}
	
int main(void)
{

DDRB = 0x03; // PB2 input for LMT87, PB0 and PB1 output

PORTB = 0x00; // first all outputs off... 

adc_init();
	
timer_init();
	
sei();

	while(1)
	{
	// thank you and good night...
	}
return 0;
}
