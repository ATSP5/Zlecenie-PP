/*
 * CzujnikEwy.c
 *
 * Created: 2017-12-12 00:25:27
 */ 


#define F_CPU 8000000UL
#include <stddef.h>
#include <math.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
volatile int16_t Dana=0,PomDana=0;

ISR (BADISR_vect)
{
	
}
ISR (ADC_vect)
{
	Dana=ADCL;
	PomDana=ADCH;
	Dana=(Dana|(PomDana<<8));
}
void ADC_INI()
{
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS0)|(1<<ADATE)|(1<<ADPS1)|(1<<ADIE)|(1<<ADSC);//128 krotny dzielnik
}


int main(void)
{
	sei();//W³¹cz przerywania
	DDRA=0;
	DDRD=255;
	PORTD=0;
	double odleglosc=0.0;
	ADC_INI();
	while(1)
	{
		cli();
		odleglosc=(double)Dana/1024*5.05;//przeliczenie na napiêcie
		odleglosc=-1.166*log(odleglosc)+4.1212;//przeliczenie napiêcia na odleg³oœæ zgodnie z charakterystyk¹.
		if(odleglosc<5.0)
		{
			PORTD=1;
		}
		if(odleglosc>5&&odleglosc<20)
		{
			PORTD=16;
		}
		if(odleglosc>20&&odleglosc<30)
		{
			PORTD=64;
		}
		if(odleglosc>30)
		{
			PORTD=32;
		}
		sei();
		_delay_ms(80);

	}
}