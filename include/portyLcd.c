#include <msp430x14x.h>

void InitPortsLcd(void)
{
    P2SEL = 0;
    P2OUT = 0;          // wyczyszczenie linii danych
    P2DIR = ~BIT0;      // ustawienie P2.0 jako wejscia, 
                        // reszty bitow jako wyjscia
}