#include "Highscores.h"
#include <msp430x14x.h>

int highscores_update(int& input, int& score)
{
  
  return 0;
}

void highscores_display()
{
  
}

int highscores(int& input, int score)
{
  int loop = 0;
  
  while(!loop)
  {
    loop = highscores_update(input, score);
    highscores_display();
    
    input = 0;
    _BIS_SR(LPM0_bits);          // wejscie w tryb oszczedny
  }
  
  return loop;
}