#include <msp430x14x.h>
#include "Lcd.h"
#include "Menu.h"
#include "Game.h"
#include "Highscores.h"

#define BUTTON_1 !(P4IN&0x10)
#define BUTTON_2 !(P4IN&0x20)
#define BUTTON_3 !(P4IN&0x40)
#define BUTTON_4 !(P4IN&0x80)

unsigned int cntr;
int input;

void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  P4DIR = ~0xf0;        // przyciski
  
  BCSCTL1 |= XTS;                     // ACLK = LFXT1 = HF XTAL 8MHz
  do
  {
    IFG1 &= ~OFIFG;                     // Czyszczenie flgi OSCFault
    for (int i = 0xFF; i > 0; i--);     // odczekanie
  }while ((IFG1 & OFIFG) == OFIFG);    // dop?ki OSCFault jest ciagle ustawiona
  BCSCTL1 |= DIVA_1;                  // ACLK = 8 MHz/2 = 4MHz
  BCSCTL2 |= SELM0 | SELM1;           // MCLK = LFTX1 = ACLK
  TACTL = TASSEL_1 + MC_1 + ID_2;     // Wybieram ACLK, tryb Up, ACLK/4 = 1MHz
  CCTL0 = CCIE;                       // wlaczenie przerwan od CCR0
  CCR0 = 10000;                       // Przerwanie generowane co 10 ms
  // 1MHz/10000 = 1/100s
  _EINT();                            // Wlaczenie przerwan
  
  int app_state = 0;
  while(1)
  {
    switch(app_state)
    {
    case 0:
      app_state = menu(input);
    case 1:
      game(input);
    case 2:
      highscores(input, 0);
    default:
      app_state = 0;
    }
  }
}

#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A(void)
{
  if (BUTTON_1)
    input = 1;
  else if (BUTTON_2)
    input = 2;
  else if (BUTTON_3)
    input = 3;
  else if (BUTTON_4)
    input = 4;
  
  if(++cntr >= 20)
  {
    cntr = 0;
    _BIC_SR_IRQ(LPM0_bits); // wyjscie z trybu oszczednego
  }
}