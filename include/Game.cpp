#include "Game.h"
#include <msp430x14x.h>
#include "Lcd.h"

bool operator==(Snake_segment& a, Snake_segment& b)
{
  if(a.x == b.x && a.y == b.y)
    return true;
  return false;
}

Snake::Snake()
{
  head = new Snake_segment(2, 0);
  head->next = new Snake_segment(1, 0);
  head->next->next = new Snake_segment(0, 0);
  
  y_direction = 0;
  x_direction = 1;
}

void Snake::display(int** bufor)
{
  Snake_segment* current = head;
  while(current)
  {
    bufor[current->x][current->y] = 1;
    current = current->next;
  }
}

int game_update(int& input, Snake* snake)       // aktualizacja stanu gry
{
  switch(input)
  {
  case 1:       // button 1
    snake->x_direction = -1;
    snake->y_direction = 0;
  case 2:       // button 2
    snake->x_direction = 1;
    snake->y_direction = 0;
  case 3:       // button 3
    snake->x_direction = 0;
    snake->y_direction = -1;
  case 4:       // button 4
    snake->x_direction = 0;
    snake->y_direction = 1;
  }
  
  snake->head->x += snake->x_direction;
  snake->head->y += snake->y_direction;
  
  if(snake->head->x > 15 || snake->head->x < 0)
    return 1;
  if(snake->head->y > 3 || snake->head->y < 0)
    return 1;
  
  Snake_segment* current = snake->head->next;
  while(current)
  {
    if(*snake->head == *current)
      return 1;
    current = current->next;
  }
  
  return 0;
}

void game_display(Snake* snake)                 // wyswietlanie gry
{
  int** bufor = new int*[16];
  for(int i = 0; i < 16; i++)
  {
    bufor[i] = new int[4];
    for(int j = 0; j < 4; j++)
      bufor[i][j] = 0;
  }
  
  snake->display(bufor);
  // display food
  
  // konwersja bufora na znaki
  
  for(int i = 0; i < 16; i++)
    delete[] bufor[i];
  delete[] bufor;
}

int game(int& input)                            // petla glowna gry
{ 
  int loop = 0;
  
  Snake* snake = new Snake;
  
  while(!loop)
  {
    loop = game_update(input, snake);
    game_display(snake);
    
    input = 0;
    _BIS_SR(LPM0_bits);          // wejscie w tryb oszczedny
  }
  
  return loop;
}