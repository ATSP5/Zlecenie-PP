/*
 * Gra.c
 *
 * Created: 2017-12-04 00:39:31
 *  Author: Adam
 */ 

#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
void Init(void)
{
	DDRA=0xFF;//Port A wyj�ciowy
	PORTA=0xC0;//Pocz�tkowo wszystkie diody wy��czone
	DDRB=0xF8;//Pierwsze 3 linie wej�cie reszta wyj�cie
	PORTB=0xFF;// Linie PB0-PB7 podci�gni�te programowo do +5V
}
void Zapal(int8_t Numer)
{
	_delay_us(10);
	PORTA=0xC0;
	switch(Numer)
	{
	case 0: case 1:
	PORTA=0xC0;
	break;
	case 2:
	PORTA=0xE9;
	break;
	case 3:
	PORTA=0xE3;
	break; 
	case 4:
	PORTA=0xCB;
	break;
	case 5:
	PORTA=0xEC;
	break;
	case 6:
	PORTA=0xE6;
	break;
	case 7:
	PORTA=0xCE;
	break;
	case 8:
	PORTA=0xF8;
	break;
	case 9:
	PORTA=0xF2;
	break;
	case 10:
	PORTA=0xDA;
	break;
	case 11:
	PORTA=0x40;
	break;
	case 12:
	PORTA=0x80;
	break;
	default:
	break;
	}
}
int8_t SprawdzKlawisz(void)
{
	bool przycisnieto=false;
	while(przycisnieto==false)//p�tla czekaj�ca na wci�ni�cie przycisku
	{
	PORTB &=~(1<<PB3);//Zerujemy PB3 mo�liwo�� odczytania stanu g�rnego wiersza klawiaury
	_delay_us(1);//Czekamy na ustalenie stanu nieustalonego
	for(int i=0; i<3; i++)//odczytaj stany wszystkich trzech przycisk�w
	{
	if(bit_is_clear(PINB,i))//sprawdzamy czy dany przycisk zwar� wysoki stan na wej�ciu do niskiego na wyj�ciu (bo zerowali�my PB3 a 
	{//PB0-2 pozostaj� w stanie wysokim.)
		_delay_ms(60);//Poczekaj 60 ms na ustalenie si� stanu nieustalonego
		if(bit_is_clear(PINB,i))// czy na pewno przycisk zosta� wci�ni�ty? Czy tylko drgania styk�w.
		{
			przycisnieto=true;//je�li tak to wyjd� z p�tli nad��dnej i zwr�� numer wci�ni�tego klawisza
			return 2+i;
		}
	}
	}
	PORTB |=(1<<PB3);//wcze�niej wyzerowan� lini� ustaw
	PORTB &=~(1<<PB4);//wyzeruj lini� PB4 w celu odczytania �rodkowego rz�du klawiatury... dalej analogicznie.
	_delay_us(1);
	for(int i=0; i<3; i++)
	{
		if(bit_is_clear(PINB,i))
		{
			_delay_ms(60);
			if(bit_is_clear(PINB,i))
			{
				przycisnieto=true;
				return 5+i;
			}
		}
	}
	PORTB |=(1<<PB4);
	PORTB &=~(1<<PB5);
	_delay_us(1);
	for(int i=0; i<3; i++)
	{
		if(bit_is_clear(PINB,i))
		{
			_delay_ms(60);
			if(bit_is_clear(PINB,i))
			{
				przycisnieto=true;
				return 8+i;
			}
		}
	}
	PORTB |=(1<<PB5);
	}
	return 0;
}
bool CzytajKlawisze(int8_t tab[],int8_t lev)
{
	bool Zgadl=false;
	for(int i=0; i<lev; i++)
	{
		if(SprawdzKlawisz()!=tab[i])//Sprawd� czy dany klawisz zgadza si� z wylosowanym w tabeli wynik�w
		{//je�li nie to przerwij p�tl� i zwr�� fa�sz
			Zgadl=false;
			break;
		}
		else//je�li tak mo�na kontynuowa� p�tl� wst�pnie zostawiamy ,,prawda" 
		{
			Zgadl=true;
		}
		_delay_ms(200);// Daj u�ytkownikowi czas na wci�ni�cie kolejnego przycisku
	}
	return Zgadl;
}
int main(void)
{
	Init();//Zainicjuj mikrokontroler
	int8_t TabelaWynikow[50],poziom=3;//Tabela wynik�w losowa� i poziom trudno�ci 
    while(1)
    {
       for(int8_t i=0; i<poziom; i++)// Losowanie di�d do zapalenia
	   {
		   TabelaWynikow[i]=rand()%9+2;// Losowanie do tabeli wynik funkcji dzielimy modulo 9 (wyniki z zakresu 0-8) i dodajemy 2 co przesuwa na zakres 2-10.
		   if(i!=0)//Ten blok odpowiedzialny za to �eby 2 s�siednie wyniki nie by�y takie same gdy� trudne jest to do zauwa�enia
		   {//objawia si� to pojedy�czym przedlu�onym �wieceniem diody zamiast dw�ch oddzielnych. Sprawdzamy czy i!=0 bo nie ma
		   if(TabelaWynikow[i]==TabelaWynikow[i-1])//elementu tabeli [-1]. Sprawdzamy czy s�siedzi nie s� sobie r�wne przerywamy 
		   {//aktualny krok p�tli poleceniem continue odejmuj�c 1 od licznika p�tli �eby nie straci� po��danej ilo�ci obrot�w p�tli.
			   --i;
			   continue;
		   }
		   }//koniec teo bloku
		   Zapal(TabelaWynikow[i]);
		   _delay_ms(500);
	   }
	   Zapal(0);
	   if(CzytajKlawisze(TabelaWynikow,poziom)==true)//je�eli nasze ,,strzaly" zgadza�y si� z tymi wylosowanymi zapal zielon�
	   {
		   Zapal(12);
		   poziom++;// zwi�ksz trudno��
		   if(poziom>=50)// je�eli poziom przekroczy 50 wy�wietl 3 razy zielon� (Gratuacje) i wr�� do poziomu 3.
		   {
			   for(int i=0; i<3; i++)
			   {
				   Zapal(12);
				   _delay_ms(350);
				   Zapal(0);
				   _delay_ms(350);
			   }
			   poziom=3;
		   }
	   }
	   else// je�eli nasz ,,strza�" by� nieudany zapal czerwon� i wr�� do poziomu 3.
	   {
		   Zapal(11);
		   poziom=3;
	   }
	   _delay_ms(1000);//oddziel nowy poziom od poprzedniego 1s przerw�.
    }
}