/*
 * Kalkulaor_Ewy.c
 *
 * Created: 2017-12-11 22:10:08
 *  Author: Ewa Tomaszewska
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <stddef.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//zdefiniuj odpowiednie sta�e port�w:
#define DDR_LCD DDRB
#define PORT_LCD PORTB
#define RS_LCD 7// Dla linii RS bit nr 7
#define RW_LCD 6// Dla linii RW bit nr 6
#define EN_LCD 5// Dla linii EN bit nr 5
#define DD4_LCD 4// Dla Linii danych bitu 4 bit nr 4
#define DD5_LCD 3// Dla Linii danych bitu 5 bit nr 3
#define DD6_LCD 2// Dla Linii danych bitu 6 bit nr 2
#define DD7_LCD 1// Dla Linii danych bitu 7 bit nr 1
#define A 0// Dla Anody diody wy�wietlacza bit nr 0
void WyslijLCD(int8_t);
void WlaczLCD(void);
void CzyscLCD(void);
void WyswietlLCD(char[],int8_t);
int8_t Insert(int8_t dat,bool FirstHalfBajt)//Ustaw odpowiednie bity linii danych (tryb 4'ro bitowy) pozostawi�j�c reszt� jako 0, dzi�ki czemu mo�na u�y� operacji sumy logicznej. 
{
	int8_t prt=0,maska=1;
	if(FirstHalfBajt==true)//je�eli ma by� przesy�any m�odszy p�bajt
	{
		if((dat&maska)==maska)//je�eli pierwszy bit zmiennej dat jest ustawiony 
		prt|=(1<<DD4_LCD);//ustaw bit odpowiadaj�cy lini DD4_LCD
		maska=maska<<1;//przesu� zawarto�� zmiennej maska (tu 1) bitowo w lewo o jeden
		if((dat&maska)==maska)//powta�aj procedur� dla wszystkich czterech bit�w.
		prt|=(1<<DD5_LCD);
		maska=maska<<1;
		if((dat&maska)==maska)
		prt|=(1<<DD6_LCD);
		maska=maska<<1;
		if((dat&maska)==maska)
		prt|=(1<<DD7_LCD);
	}
	else//je�eli ma by� ustawiony starszy p�bajt
	{
		dat=dat>>4;//przesu� bitowo w prawo zawarto�� zmiennej dat o 4 bity w celu ustawienia starszego p�bajta jako m�odszy
		if((dat&maska)==maska)//powt�rz procedur� j.w.
		prt|=(1<<DD4_LCD);
		maska=maska<<1;
		if((dat&maska)==maska)
		prt|=(1<<DD5_LCD);
		maska=maska<<1;
		if((dat&maska)==maska)
		prt|=(1<<DD6_LCD);
		maska=maska<<1;
		if((dat&maska)==maska)
		prt|=(1<<DD7_LCD);
	}
	return prt;//port LCD
}
int8_t CheckBuf(char txt[],bool dot)
{
	int8_t q=0;//pomocniczy licznik
	for(int8_t i=0; i<16; i++)
	{
		if(txt[i]==0||(dot==false&&txt[i]=='.'))//je�eli w tablicy txt jest 0 lub kropka kt�rej ma nie by�
		break;//przerwij p�tl�
		q=i;//ustaw pomocniczy, g��wnym licznikiem p�tli
		q++;//zwi�ksz go o 1
	}
	if(q==0)//Zawsze zwr�� przynajmniej 1 �eby wy�wietlacz wy�wietli� pierwszy znak
	return 1;
	else
	return q;//Zwr�� liczb� wanych danych w buforze
}
void Init(void)
{
	DDRA=0xF0;//Pierwsze 4 linie wej�cie reszta wyj�cie
	PORTA=0xFF;// Linie PA0-PA7 podci�gni�te programowo do +5V
}
int8_t SprawdzKlawisz(void)
{
	bool przycisnieto=false;
	while(przycisnieto==false)//p�tla czekaj�ca na wci�ni�cie przycisku
	{
		PORTA &=~(1<<PA4);//Zerujemy PA4 mo�liwo�� odczytania stanu g�rnego wiersza klawiaury
		_delay_us(1);//Czekamy na ustalenie stanu nieustalonego
		for(int i=0; i<4; i++)//odczytaj stany wszystkich czterech przycisk�w
		{
			if(bit_is_clear(PINA,i))//sprawdzamy czy dany przycisk zwar� wysoki stan na wej�ciu do niskiego na wyj�ciu (bo zerowali�my PB3 a
			{//PB0-2 pozostaj� w stanie wysokim.)
				_delay_ms(120);//Poczekaj 120 ms na ustalenie si� stanu nieustalonego
				if(bit_is_clear(PINA,i))// czy na pewno przycisk zosta� wci�ni�ty? Czy tylko drgania styk�w.
				{
					przycisnieto=true;//je�li tak to wyjd� z p�tli nad��dnej i zwr�� numer wci�ni�tego klawisza
					return i;
				}
			}
		}
		PORTA |=(1<<PA4);//wcze�niej wyzerowan� lini� ustaw
		PORTA &=~(1<<PA5);//wyzeruj lini� PB4 w celu odczytania kolejnego rz�du klawiatury... dalej analogicznie.
		_delay_us(1);
		for(int i=0; i<4; i++)
		{
			if(bit_is_clear(PINA,i))
			{
				_delay_ms(120);
				if(bit_is_clear(PINA,i))
				{
					przycisnieto=true;
					return 4+i;
				}
			}
		}
		PORTA |=(1<<PA5);
		PORTA &=~(1<<PA6);
		_delay_us(1);
		for(int i=0; i<4; i++)
		{
			if(bit_is_clear(PINA,i))
			{
				_delay_ms(120);
				if(bit_is_clear(PINA,i))
				{
					przycisnieto=true;
					return 8+i;
				}
			}
		}
		PORTA |=(1<<PA6);
		PORTA &=~(1<<PA7);
		_delay_us(1);
		for(int i=0; i<4; i++)
		{
			if(bit_is_clear(PINA,i))
			{
				_delay_ms(120);
				if(bit_is_clear(PINA,i))
				{
					przycisnieto=true;
					return 12+i;
				}
			}
		}
		PORTA |=(1<<PA7);
	}
	return 16;
}


int main(void)
{
	_delay_us(200);
	Init();
	WlaczLCD();
	bool druga=false,kropka=false;
    double LiczbaA=0.0, LiczbaB=0.0, Wynik=0.0;
	int8_t klawisz=16,operacja=0;
	double LicznikKropki=1.0;
	char bufor [16]={0},hellotxt[5]="Hello";
	WyswietlLCD(hellotxt,5);
	_delay_ms(1000);
	CzyscLCD();
	dtostrf(LiczbaA,10,8,bufor);//Zamie� liczb� typu double na ci�g znak�w do wy�wietlenia
	 WyswietlLCD(bufor,CheckBuf(bufor,false));//wy�wietl ci�g znak�w
    while(1)
    {	
        klawisz=SprawdzKlawisz();// Sprawd� jaki klawisz jest wci�ni�ty
		switch(klawisz)
		{
			case 0: //Je�eli klawisz 0 to:
			if(druga==false)//Je�eli wpisujemy pierwsz� liczb� 
			{
				
				if(kropka==false)// Je�eli nie jest to liczba po kropce
				{
					LiczbaA=LiczbaA*10;//Przesu� liczb� w lewo (wymn� przez 10) 
					LiczbaA+=1;// Dodaj liczb� odpowiadaj�c� wci�ni�temu klawiszowi tu 7.
				}
				else//A je�eli jest to liczba po kropce
				{
					LiczbaA+=(double)1/pow(10.0,LicznikKropki);//Dodaj liczb� odpowiadaj�c� wci�ni�temu klawiszowi tu 1, przesuni�t� i tyle miejsc w prawo
					//od przecinka ile odnotowano wci�ni�� danego klawisza
				}
				dtostrf(LiczbaA,10,8,bufor);//Zamie� liczb� typu double na ci�g znak�w do wy�wietlenia
				CzyscLCD();
                 WyswietlLCD(bufor,CheckBuf(bufor,kropka));//wy�wietl ci�g znak�w
			}
			else// analogicznie jak wy�ej dla drugiej liczby...
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=1;
				}
				else
				{
					LiczbaB+=(double)1/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 1://Analogicznie j.w. dla kolejnych cyfr
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=4;
					
				}
				else
				{
					LiczbaA+=(double)4/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				 WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{

				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=4;
				}
				else
				{
					LiczbaB+=(double)4/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 2:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=7;
				}
				else
				{
					LiczbaA+=(double)7/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=7;
				}
				else
				{
					LiczbaB+=(double)7/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 12: //operacja mno�enia nr 12
			operacja=1;//Operacja nr 1 ty mno�enie
			druga=true;//Zaznacz �e od teraz wpisujemy drug� liczb�
			LicznikKropki=1;//licznik kropki znowu od 1
			kropka=false;// wpisujemy now� liczb� nie musi to by� liczba po przecinku
			CzyscLCD();
			dtostrf(LiczbaB,10,8,bufor);
			CzyscLCD();
			WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			break;
			case 4:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=2;
				}
				else
				{
					LiczbaA+=(double)2/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=2;
				}
				else
				{
					LiczbaB+=(double)2/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 5:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=5;
				}
				else
				{
					LiczbaA+=(double)5/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=5;
				}
				else
				{
					LiczbaB+=(double)5/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 6:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=8;
				}
				else
				{
					LiczbaA+=(double)8/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=8;
				}
				else
				{
					LiczbaB+=(double)8/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				 WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 11://odejmowanie
			operacja=2;//operacja nr 2
			druga=true;//zaznacz �e od teraz wpisujemy druga liczb�
			LicznikKropki=1;//w przypadku liczby po przecinku wpisuj od pierwszego miejsca
			kropka=false;//zacznij od liczb za�kowitych
			CzyscLCD();
			dtostrf(LiczbaB,10,8,bufor);
			CzyscLCD();
			WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			break;
			case 8:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=3;
				}
				else
				{
					LiczbaA+=(double)3/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=3;
				}
				else
				{
					LiczbaB+=(double)3/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				 WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 9:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=6;
				}
				else
				{
					LiczbaA+=(double)6/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=6;
				}
				else
				{
					LiczbaB+=(double)6/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			break;
			case 10:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=9;
				}
				else
				{
					LiczbaA+=(double)9/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=9;
				}
				else
				{
					LiczbaB+=(double)9/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			break;
			case 13:
			operacja=3;//dzielenie
			druga=true;
			LicznikKropki=1;
			kropka=false;
			CzyscLCD();
			dtostrf(LiczbaB,10,8,bufor);
			CzyscLCD();
			WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			break;
			case 7:
			if(druga==false)
			{
				
				if(kropka==false)
				{
					LiczbaA=LiczbaA*10;
					LiczbaA+=0;
				}
				else
				{
					LiczbaA+=(double)0/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaA,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			else
			{
				
				if(kropka==false)
				{
					LiczbaB=LiczbaB*10;
					LiczbaB+=0;
				}
				else
				{
					LiczbaB+=(double)0/pow(10.0,LicznikKropki);
				}
				dtostrf(LiczbaB,10,8,bufor);
				CzyscLCD();
				WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			}
			if(kropka==true)
			LicznikKropki++;
			break;
			case 3://obs�uga kropki (liczby zmienoprzecinkowe
			kropka=true;//zaznacz �e kolejno wpisane liczby maj� by� po przecinku
			break;
			case 15://(r�wna si�) wy�wietl wynik operacji
			Wynik=0.0;//zeruj wynik
			switch(operacja)
			{
				case 1:
				Wynik=LiczbaA*LiczbaB;
				break;
				case 2:
				Wynik=LiczbaA-LiczbaB;
				break;
				case 3:
				Wynik=LiczbaA/LiczbaB;
				break;
				case 4:
				Wynik=LiczbaA+LiczbaB;
				break;
			}
			CzyscLCD();
			dtostrf(Wynik,7,5,bufor);
			 WyswietlLCD(bufor,CheckBuf(bufor,true));
			break;
			case 14://dodawanie
			operacja=4;
			druga=true;
			LicznikKropki=1;
			kropka=false;
			CzyscLCD();
			dtostrf(LiczbaB,10,8,bufor);
			CzyscLCD();
			WyswietlLCD(bufor,CheckBuf(bufor,kropka));
			break;
			default:
			break;
		}
		
    }
}
void WyslijLCD(int8_t bajtmask)
{
	int8_t maska=255;//Chcemy mie� mask� ze wszystkimi ustawionymi bitami
	maska &= ~(1<<DD4_LCD);//zerujemy bity odpowiadaj�ce liniom danych 
	maska &= ~(1<<DD5_LCD);//
	maska &= ~(1<<DD6_LCD);//
	maska &= ~(1<<DD7_LCD);//
	PORT_LCD=PORT_LCD&maska;//Operacja AND powoduje wyzerowanie bit�w portu tam gdzie w masce by�y zera a pozostawienie bez zmian
	//tam gdzie by�y jedynki (w masce)
	PORT_LCD |= (1<<EN_LCD);//Ustawiamy bit EN w celu przes�ania wa�nych danych do wy�wietlacza
	PORT_LCD|= Insert(bajtmask,false);//Ustawiamy pierwszy p�bajt (starszy) na porcie LCD (operacja sumy logicznej dla tego musz� tam by�
	//same zera.)
	_delay_us(10);//Op�nienie linii
	PORT_LCD &= ~(_BV(EN_LCD));// Ko�czymy transmisj� pierwszego p�bajtu
	_delay_us(1);// Op�nienie lini
	PORT_LCD=PORT_LCD&maska;//Ponownie zerujemy lini� danych
	PORT_LCD |= (1<<EN_LCD);//Ustawiamy EN w celu rozpocz�cia wa�nej transmisji danych
	PORT_LCD|= Insert(bajtmask,true);// Ustawiamy pierwszy p�bajt (m�odszy) na porcie LCD (operacja sumy logicznej dla tego musz� tam by�
	//same zera.)
	_delay_us(10);//Op�nienie linii
	PORT_LCD &=~ (_BV(EN_LCD));// Koniec transmisji zerujemy bit EN
	_delay_us(40);// op�nienie linii
	
	
}
void CzyscLCD()
{
	
	PORT_LCD &= ~(_BV(RS_LCD));//Zerujemy RS aby mie� dost�p do polece� wy�wietlacza (transmisja nie wysy�a danych do wy�wietlenia)
	WyslijLCD(1);//Czy��
	PORT_LCD |= (_BV(RS_LCD));// Wr�� do trybu transmisji w celu wy�wietlania
	_delay_ms(2);
	
}

void WyswietlLCD(char napis[],int8_t ile)
{
	
	int8_t k=0,i=0;
	while(k<ile)// Wy�wietl po kolei kazdy znak znajduj�cy si� w tablicy
	{
		i=(int8_t)napis[k];//rzutuj typ znakowy na jego reprezentacj� w formie liczby
		WyslijLCD(i);//Wy�lij do LCD
		k++;
	}
	
}
void WlaczLCD()
{
	DDR_LCD=255;//Port LCD jako wyj�cie
	PORT_LCD=0;//Pocz�tkowy stan zerowy linii
	_delay_ms(100);//op�nienie linii
	PORT_LCD|=(1<<A);//w��cz LED
	PORT_LCD&=~(1<<RW_LCD);
	PORT_LCD&=~(_BV(RS_LCD));//Tryb polece� wy�wietlacza
	//Dalej procedura inicjalizacji wy�wietlacza (patrz nota katalogowa HD 44780 wy�wietlanie w trybie 4'ro bitowym)
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