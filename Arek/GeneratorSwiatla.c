/*
 * GeneratorSwiatla.c
 *
 * Created: 2018-01-18 15:43:56
 *  Author: Adam
 */ 



#define F_CPU 8000000UL
#include <stddef.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#define DDR_LCD DDRB
#define PORT_LCD PORTB
#define A      0
#define RW_LCD 1
#define RS_LCD 2
#define EN_LCD 3
#define DD4_LCD 4
#define DD5_LCD 5
#define DD6_LCD 6
#define DD7_LCD 7
#include <avr/io.h>
volatile int16_t Dana=0,PomDana=0;
volatile bool We=true;
double procentUstawiony=0.0;
volatile int8_t TimerCounter=0;

void WyslijLCD(int8_t);
void WlaczLCD(void);
void CzyscLCD(void);
void WyswietlLCD(char[],int8_t);
void TimerInit() //Musi być opatrzona cli i sei
{
	cli();
	unsigned char sreg;
	sreg=SREG;
	TCCR1B |= (1<<WGM12)|(1<<CS11);//tryb ctc preskaler 1
	OCR1A = 7;
	TIMSK |= (1<<OCIE1A); //zezwolenie na przerwanie
	SREG=sreg;
	TCNT1=0;
	sei();
}
ISR (BADISR_vect)
{
	
}
ISR (TIMER1_COMPA_vect)
{
	if(TimerCounter==100)
	{
		TimerCounter=0;
	}
	TimerCounter++;
	if(procentUstawiony>=TimerCounter)
	PORTA|=(1<<2);
	else
	PORTA&=~(1<<2);
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
	ADCSRA |= (1<<ADEN)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADATE)|(1<<ADIE)|(1<<ADSC);//
}

int main(void)
{
	DDRA=4;
	PORTA=0;
	char bufor[10];
	double ADC0=0.0,ADC1=0.0,procent=0.0;
	char txt[8]="Moc[%]: ", hellotxt[5]="Hello";
	WlaczLCD();
	WyswietlLCD(hellotxt,5);
	_delay_ms(1000);
	CzyscLCD();
	ADC_INI();
	TimerInit();
	while(1)
	{
		_delay_ms(125);
		cli();
		if(We==true)//Jeśli przetważanie pochodzi z kanału 0
		{
			ADC0=(double)Dana/1024*5.05;
			procentUstawiony=(double) (ADC0*100.0)/5.05;
			ADMUX|=(1<<MUX1);//Przełącz na kanał ADC1
		}
		else//Jeśli przetwarzanie pochodzi z kanału 1
		{
			ADC1=(double)Dana/1024*5.05;
			procent=(double) 48.077*ADC1-125;
			if(procent<0)
			procent=0;
			CzyscLCD();
			dtostrf(procent,7,5,bufor);
			WyswietlLCD(txt,8);
			WyswietlLCD(bufor,7);
			ADMUX&=~(1<<MUX1);//Przełącz na kanał ADC0
		}
		if(We==true)
		{
			We=false;	
		}
		else
		{
			We=true;
		}
		sei();
	}
}
void WyslijLCD(int8_t bajtmask)
{
	
	PORT_LCD |= (1<<EN_LCD);
	PORT_LCD = (bajtmask&0xF0)|(PORT_LCD&0x0F);
	_delay_us(10);//Opóźnienie linii===
	PORT_LCD &= ~(_BV(EN_LCD));
	_delay_us(1);
	PORT_LCD |= (1<<EN_LCD);
	PORT_LCD = ((bajtmask&0x0F)<<4)|(PORT_LCD&0x0F);
	_delay_us(10);//Opóźnienie linii
	PORT_LCD &=~ (_BV(EN_LCD));
	_delay_us(40);
}
void CzyscLCD()
{
	
	PORT_LCD &= ~(_BV(RS_LCD));
	WyslijLCD(1);
	PORT_LCD |= (_BV(RS_LCD));
	_delay_ms(2);
	
}

void WyswietlLCD(char napis[],int8_t ile)
{
	
	int8_t k=0,i=0;
	while(k<ile)
	{
		i=(int8_t)napis[k];
		WyslijLCD(i);
		k++;
	}
	
}
void WlaczLCD()
{
	
	DDR_LCD=255;
	PORT_LCD=0;
	_delay_ms(100);
	PORT_LCD&=~(_BV(RS_LCD));//modyf
	//Wyślij sekwencję 0011
	for(int i=0;i<3;i=i+1)
	{
		PORT_LCD |= (1<<EN_LCD);
		PORT_LCD |= (1<<DD4_LCD)|(1<<DD5_LCD);
		_delay_us(10);//Opóźnienie linii===
		PORT_LCD &= ~(_BV(EN_LCD));
		_delay_us(100);
	}
	//4 bitowy interfejs
	PORT_LCD |= (1<<EN_LCD);
	PORT_LCD &= ~(_BV(DD4_LCD));
	_delay_us(10);//Opóźnienie linii===
	PORT_LCD &= ~(_BV(EN_LCD));
	//Parametry Wyświetlacza
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00101000);
	PORT_LCD|=_BV(RS_LCD);
	_delay_us(1);
	//Parametry Wyświetlacza
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00000110);
	PORT_LCD|=_BV(RS_LCD);
	_delay_us(1);
	//Włącz wyświetlacz
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00001110);
	PORT_LCD|=_BV(RS_LCD);
	_delay_ms(1);
	//Czyść wyświetlacz
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00000001);
	PORT_LCD|=_BV(RS_LCD);
	_delay_ms(2);
	
}