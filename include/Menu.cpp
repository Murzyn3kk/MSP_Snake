#include "Menu.h"
#include <msp430x14x.h>

int menu_update(int& input)     // aktualizacja stanu menu
{
  switch(input)
  {
  case 1:
    return 1;
  }
  
  return 0;
}

void menu_display()             // wyswietlanie menu
{
  
}

int menu(int& input)            // petla glowna menu
{ 
  int loop = 0;
  
  while(!loop)
  {
    loop = menu_update(input);
    menu_display();
    
    input = 0;
    _BIS_SR(LPM0_bits);          // wejscie w tryb oszczedny
  }

  return loop;
}