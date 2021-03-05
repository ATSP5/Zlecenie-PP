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
//zdefiniuj odpowiednie sta³e portów:
#define DDR_LCD DDRB
#define PORT_LCD PORTB
#define RS_LCD 7// Dla linii RS bit nr 7
#define RW_LCD 6// Dla linii RW bit nr 6
#define EN_LCD 5// Dla linii EN bit nr 5
#define DD4_LCD 4// Dla Linii danych bitu 4 bit nr 4
#define DD5_LCD 3// Dla Linii danych bitu 5 bit nr 3
#define DD6_LCD 2// Dla Linii danych bitu 6 bit nr 2
#define DD7_LCD 1// Dla Linii danych bitu 7 bit nr 1
#define A 0// Dla Anody diody wyœwietlacza bit nr 0
void WyslijLCD(int8_t);
void WlaczLCD(void);
void CzyscLCD(void);
void WyswietlLCD(char[],int8_t);
int8_t Insert(int8_t dat,bool FirstHalfBajt)//Ustaw odpowiednie bity linii danych (tryb 4'ro bitowy) pozostawi¹j¹c resztê jako 0, dziêki czemu mo¿na u¿yæ operacji sumy logicznej. 
{
	int8_t prt=0,maska=1;
	if(FirstHalfBajt==true)//je¿eli ma byæ przesy³any m³odszy pó³bajt
	{
		if((dat&maska)==maska)//je¿eli pierwszy bit zmiennej dat jest ustawiony 
		prt|=(1<<DD4_LCD);//ustaw bit odpowiadaj¹cy lini DD4_LCD
		maska=maska<<1;//przesuñ zawartoœæ zmiennej maska (tu 1) bitowo w lewo o jeden
		if((dat&maska)==maska)//powta¿aj procedurê dla wszystkich czterech bitów.
		prt|=(1<<DD5_LCD);
		maska=maska<<1;
		if((dat&maska)==maska)
		prt|=(1<<DD6_LCD);
		maska=maska<<1;
		if((dat&maska)==maska)
		prt|=(1<<DD7_LCD);
	}
	else//je¿eli ma byæ ustawiony starszy pó³bajt
	{
		dat=dat>>4;//przesuñ bitowo w prawo zawartoœæ zmiennej dat o 4 bity w celu ustawienia starszego pó³bajta jako m³odszy
		if((dat&maska)==maska)//powtórz procedurê j.w.
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
		if(txt[i]==0||(dot==false&&txt[i]=='.'))//je¿eli w tablicy txt jest 0 lub kropka której ma nie byæ
		break;//przerwij pêtlê
		q=i;//ustaw pomocniczy, g³ównym licznikiem pêtli
		q++;//zwiêksz go o 1
	}
	if(q==0)//Zawsze zwróæ przynajmniej 1 ¿eby wyœwietlacz wyœwietli³ pierwszy znak
	return 1;
	else
	return q;//Zwróæ liczbê wanych danych w buforze
}
void Init(void)
{
	DDRA=0xF0;//Pierwsze 4 linie wejœcie reszta wyjœcie
	PORTA=0xFF;// Linie PA0-PA7 podci¹gniête programowo do +5V
}
int8_t SprawdzKlawisz(void)
{
	bool przycisnieto=false;
	while(przycisnieto==false)//pêtla czekaj¹ca na wciœniêcie przycisku
	{
		PORTA &=~(1<<PA4);//Zerujemy PA4 mo¿liwoœæ odczytania stanu górnego wiersza klawiaury
		_delay_us(1);//Czekamy na ustalenie stanu nieustalonego
		for(int i=0; i<4; i++)//odczytaj stany wszystkich czterech przycisków
		{
			if(bit_is_clear(PINA,i))//sprawdzamy czy dany przycisk zwar³ wysoki stan na wejœciu do niskiego na wyjœciu (bo zerowaliœmy PB3 a
			{//PB0-2 pozostaj¹ w stanie wysokim.)
				_delay_ms(120);//Poczekaj 120 ms na ustalenie siê stanu nieustalonego
				if(bit_is_clear(PINA,i))// czy na pewno przycisk zosta³ wciœniêty? Czy tylko drgania styków.
				{
					przycisnieto=true;//jeœli tak to wyjdŸ z pêtli nad¿êdnej i zwróæ numer wciœniêtego klawisza
					return i;
				}
			}
		}
		PORTA |=(1<<PA4);//wczeœniej wyzerowan¹ liniê ustaw
		PORTA &=~(1<<PA5);//wyzeruj liniê PB4 w celu odczytania kolejnego rzêdu klawiatury... dalej analogicznie.
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
	dtostrf(LiczbaA,10,8,bufor);//Zamieñ liczbê typu double na ci¹g znaków do wyœwietlenia
	 WyswietlLCD(bufor,CheckBuf(bufor,false));//wyœwietl ci¹g znaków
    while(1)
    {	
        klawisz=SprawdzKlawisz();// SprawdŸ jaki klawisz jest wciœniêty
		switch(klawisz)
		{
			case 0: //Je¿eli klawisz 0 to:
			if(druga==false)//Je¿eli wpisujemy pierwsz¹ liczbê 
			{
				
				if(kropka==false)// Je¿eli nie jest to liczba po kropce
				{
					LiczbaA=LiczbaA*10;//Przesuñ liczbê w lewo (wymnó¿ przez 10) 
					LiczbaA+=1;// Dodaj liczbê odpowiadaj¹c¹ wciœniêtemu klawiszowi tu 7.
				}
				else//A je¿eli jest to liczba po kropce
				{
					LiczbaA+=(double)1/pow(10.0,LicznikKropki);//Dodaj liczbê odpowiadaj¹c¹ wciœniêtemu klawiszowi tu 1, przesuniêt¹ i tyle miejsc w prawo
					//od przecinka ile odnotowano wciœniêæ danego klawisza
				}
				dtostrf(LiczbaA,10,8,bufor);//Zamieñ liczbê typu double na ci¹g znaków do wyœwietlenia
				CzyscLCD();
                 WyswietlLCD(bufor,CheckBuf(bufor,kropka));//wyœwietl ci¹g znaków
			}
			else// analogicznie jak wy¿ej dla drugiej liczby...
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
			case 12: //operacja mno¿enia nr 12
			operacja=1;//Operacja nr 1 ty mno¿enie
			druga=true;//Zaznacz ¿e od teraz wpisujemy drug¹ liczbê
			LicznikKropki=1;//licznik kropki znowu od 1
			kropka=false;// wpisujemy now¹ liczbê nie musi to byæ liczba po przecinku
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
			druga=true;//zaznacz ¿e od teraz wpisujemy druga liczbê
			LicznikKropki=1;//w przypadku liczby po przecinku wpisuj od pierwszego miejsca
			kropka=false;//zacznij od liczb za³kowitych
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
			case 3://obs³uga kropki (liczby zmienoprzecinkowe
			kropka=true;//zaznacz ¿e kolejno wpisane liczby maj¹ byæ po przecinku
			break;
			case 15://(równa siê) wyœwietl wynik operacji
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
	int8_t maska=255;//Chcemy mieæ maskê ze wszystkimi ustawionymi bitami
	maska &= ~(1<<DD4_LCD);//zerujemy bity odpowiadaj¹ce liniom danych 
	maska &= ~(1<<DD5_LCD);//
	maska &= ~(1<<DD6_LCD);//
	maska &= ~(1<<DD7_LCD);//
	PORT_LCD=PORT_LCD&maska;//Operacja AND powoduje wyzerowanie bitów portu tam gdzie w masce by³y zera a pozostawienie bez zmian
	//tam gdzie by³y jedynki (w masce)
	PORT_LCD |= (1<<EN_LCD);//Ustawiamy bit EN w celu przes³ania wa¿nych danych do wyœwietlacza
	PORT_LCD|= Insert(bajtmask,false);//Ustawiamy pierwszy pó³bajt (starszy) na porcie LCD (operacja sumy logicznej dla tego musz¹ tam byæ
	//same zera.)
	_delay_us(10);//OpóŸnienie linii
	PORT_LCD &= ~(_BV(EN_LCD));// Koñczymy transmisjê pierwszego pó³bajtu
	_delay_us(1);// OpóŸnienie lini
	PORT_LCD=PORT_LCD&maska;//Ponownie zerujemy liniê danych
	PORT_LCD |= (1<<EN_LCD);//Ustawiamy EN w celu rozpoczêcia wa¿nej transmisji danych
	PORT_LCD|= Insert(bajtmask,true);// Ustawiamy pierwszy pó³bajt (m³odszy) na porcie LCD (operacja sumy logicznej dla tego musz¹ tam byæ
	//same zera.)
	_delay_us(10);//OpóŸnienie linii
	PORT_LCD &=~ (_BV(EN_LCD));// Koniec transmisji zerujemy bit EN
	_delay_us(40);// opóŸnienie linii
	
	
}
void CzyscLCD()
{
	
	PORT_LCD &= ~(_BV(RS_LCD));//Zerujemy RS aby mieæ dostêp do poleceñ wyœwietlacza (transmisja nie wysy³a danych do wyœwietlenia)
	WyslijLCD(1);//Czyœæ
	PORT_LCD |= (_BV(RS_LCD));// Wróæ do trybu transmisji w celu wyœwietlania
	_delay_ms(2);
	
}

void WyswietlLCD(char napis[],int8_t ile)
{
	
	int8_t k=0,i=0;
	while(k<ile)// Wyœwietl po kolei kazdy znak znajduj¹cy siê w tablicy
	{
		i=(int8_t)napis[k];//rzutuj typ znakowy na jego reprezentacjê w formie liczby
		WyslijLCD(i);//Wyœlij do LCD
		k++;
	}
	
}
void WlaczLCD()
{
	DDR_LCD=255;//Port LCD jako wyjœcie
	PORT_LCD=0;//Pocz¹tkowy stan zerowy linii
	_delay_ms(100);//opóŸnienie linii
	PORT_LCD|=(1<<A);//w³¹cz LED
	PORT_LCD&=~(1<<RW_LCD);
	PORT_LCD&=~(_BV(RS_LCD));//Tryb poleceñ wyœwietlacza
	//Dalej procedura inicjalizacji wyœwietlacza (patrz nota katalogowa HD 44780 wyœwietlanie w trybie 4'ro bitowym)
	//Wyœlij sekwencjê 0011
	for(int i=0;i<3;i=i+1)
	{
		PORT_LCD |= (1<<EN_LCD);
		
		PORT_LCD |= (1<<DD4_LCD)|(1<<DD5_LCD);
		_delay_us(10);//OpóŸnienie linii===
		PORT_LCD &= ~(_BV(EN_LCD));
		
		_delay_us(100);
	}
	//4 bitowy interfejs
	PORT_LCD |= (1<<EN_LCD);
	PORT_LCD &= ~(_BV(DD4_LCD));
	_delay_us(10);//OpóŸnienie linii===
	PORT_LCD &= ~(_BV(EN_LCD));
	//Parametry Wyœwietlacza
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00101000);
	PORT_LCD|=_BV(RS_LCD);
	_delay_us(1);
	//Parametry Wyœwietlacza
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00000110);
	PORT_LCD|=_BV(RS_LCD);
	_delay_us(1);
	//W³¹cz wyœwietlacz
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00001110);
	PORT_LCD|=_BV(RS_LCD);
	_delay_ms(1);
	//Czyœæ wyœwietlacz
	PORT_LCD&=~(_BV(RS_LCD));
	WyslijLCD(0b00000001);
	PORT_LCD|=_BV(RS_LCD);
	_delay_ms(2);
	
}