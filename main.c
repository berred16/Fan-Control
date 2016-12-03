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
for a power supply.
ADC input: 
5...2,244V - off 
2,244V...2,095V - 6V  operation  29° 2,244 ADC: 459
2,095V or less - 12V operation   40° 2,095 ADC: 429

Version 1.0d / 2016, Nov. 23th
1.0d: change temperature range
Hardware setup:

Attiny13 (DIP8 package, 8 pins)

				PB5 |1------U------8| VCC 5V
				PB3 |2             7| PB2 ADC1 LMT87 <-
				PB4 |3             6| PB1 6V oper. ->
				GND |4-------------5| PB0 12V oper. ->

Fuses:
High Fuse: FF
Low Fuse:  7A
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 9600000UL

#define SIX_V 459 // 2,244V, 29°
#define TWELVE_V 429 // 2,095V, 40°

void adc_init(void)
	{
		// ADC1, AREF=VCC (5V)
		ADMUX = (0<<REFS0)|(0<<MUX1)|(1<<MUX0); 
		// ADC enable, interrupt enable, prescaler 1/128 
		ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); 
	}

void timer_init(void)
	{
		TIMSK0 |= (1 << TOIE0); // Timer interrupts enable
		TCCR0B = (1<<CS02)|(0<<CS01)|(1<<CS00);
	}

// variables for temperatur check
char i;
uint16_t temperature;

void check_temperature(void)
{


if (temperature <= SIX_V) // equals to 29°C
	{
		if (temperature >= TWELVE_V) // equals to 40°C
			{
				PORTB = 0x01; // 6V operation between 29°C and 40°C
			}
	}

if (temperature > SIX_V) // fans off when < 29°C
	{
		PORTB = 0x00; // all fans off 
	}

if (temperature < TWELVE_V) // fans on when > 40°C
	{
		PORTB = 0x02; // 12V operation if more than 40°C	
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
	
temperature = 500; // to switch off the fans first

sei();

	while(1)
	{
	// thank you and good night...
	}
return 0;
}
