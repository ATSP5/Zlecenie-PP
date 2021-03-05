/*
 * Termometr_Ani.c
 *
 */ 

#define F_CPU 8000000UL
#include <stddef.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
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
volatile int16_t Dana=0,PomDana=0;

void WyslijLCD(int8_t);
void WlaczLCD(void);
void CzyscLCD(void);
void WyswietlLCD(char[],int8_t);
ISR (BADISR_vect)//Obs�uga nieobs�ugiwanych wektor�w przerywa�
{
	
}
ISR (ADC_vect)//Obs�uga przeywania ADC
{
	Dana=ADCL;//M�odsza cz�� 10bitowego rejestru ADC
	PomDana=ADCH;//Starsza cz�� 10bitowego rejestru ADC
	Dana=(Dana|(PomDana<<8)); //Z��czenie obu tych po��wek
}
void ADC_INI()
{
	ADMUX |= (1<<REFS0);//Napi�cie odniesienia: Napi�cie zasilania
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADATE)|(1<<ADIE)|(1<<ADSC);//128 krotny dzielnik (ustawienia ADC)
}


int main(void)
{
	sei();//W��cz przerywania 
	DDRA=0;//Port A na We
	DDRD=255;//Port D Wy
	PORTD=0;
	WlaczLCD();//W��cz LCD
    char bufor[10];//Bufor znakowy
    double temp=0.0;
	char txt[7]="Temp.: ",txt2[4]="st C";
    ADC_INI();
    while(1)
    {
       cli();//Wy��cz przerywania globalnie (Pauza w konwersji ADC, �eby przrywanie nie zak��ci�o transferu z LCD)
       CzyscLCD();
       temp=(double)Dana/1024*5.05;//Tu przeliczenie napi�cia na temperatur� wymagana kalibracja.
	   temp=23.544*temp-16.395;//Skalibrowane dane 
       dtostrf(temp,7,5,bufor);//Konwertuj typ double na typ char []
       WyswietlLCD(txt,7);//Wy�wietl tekst
       WyswietlLCD(bufor,5);//Wy�wietl zawarto�� pomocniczego bufora
	   WyswietlLCD(txt2,4);
       //Za�o�enie LED2 niebieska LED3 zielona LED4 czerwona
	   if(temp<18.0)//Zapal ledy zale�nie od odczytanej temperatury
	   {
		   PORTD=32;//
	   }
	   if(temp>18.0&&temp<25.0)
	   {
		   PORTD=8;
	   }
	   if(temp>25.0)
	   {
		   PORTD=2;
	   }
       sei();//W��cz przerywania globalnie (Umo�liwienie konwersji ADC)
       _delay_ms(250);

    }
}
void WyslijLCD(int8_t bajtmask)
{
	
	PORT_LCD |= (1<<EN_LCD);//Pocz�tek transmisji danych
	PORT_LCD = (bajtmask&0xF0)|(PORT_LCD&0x0F);//Wy�lij pierwszy p�bajt
	_delay_us(10);//Op�nienie linii===
	PORT_LCD &= ~(_BV(EN_LCD));//koniec transmisji danych
	_delay_us(1);
	PORT_LCD |= (1<<EN_LCD);//Pocz�tek transmisji danych
	PORT_LCD = ((bajtmask&0x0F)<<4)|(PORT_LCD&0x0F);//Wy�lij drugi p�bajt
	_delay_us(10);//Op�nienie linii
	PORT_LCD &=~ (_BV(EN_LCD));//koniec transmisji danych
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
	PORT_LCD |=(1<<A);
	_delay_ms(100);
	PORT_LCD&=~(_BV(RS_LCD));//modyf
	//Wy�lij sekwencj� 0011
	for(int i=0;i<3;i=i+1)
	{
		PORT_LCD |= (1<<EN_LCD);
		PORT_LCD |= (1<<DD4_LCD)|(1<<DD5_LCD);
		_delay_us(10);//Op�nienie linii===
		PORT_LCD &= ~(_BV(EN_LCD));
		_delay_us(100);
	}
	//4 bitowy interfejs
	PORT_LCD |= (1<<EN_LCD);
	PORT_LCD &= ~(_BV(DD4_LCD));
	_delay_us(10);//Op�nienie linii===
	PORT_LCD &= ~(_BV(EN_LCD));
	//Parametry Wy�wietlacza
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00101000);
	PORT_LCD|=_BV(RS_LCD);
	_delay_us(1);
	//Parametry Wy�wietlacza
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00000110);
	PORT_LCD|=_BV(RS_LCD);
	_delay_us(1);
	//W��cz wy�wietlacz
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00001110);
	PORT_LCD|=_BV(RS_LCD);
	_delay_ms(1);
	//Czy�� wy�wietlacz
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00000001);
	PORT_LCD|=_BV(RS_LCD);
	_delay_ms(2);
	
}