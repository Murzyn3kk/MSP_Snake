#pragma once

struct Snake_segment
{
  Snake_segment(int xa, int xy)
    : x(xa), y(xy) { next = 0; }
  
  int x;
  int y;
  Snake_segment* next;
};

class Snake
{
public:
  Snake();
  
  void display(int** bufor);
  
  int x_direction;
  int y_direction;
  Snake_segment* head;
};

int game_update(int&, Snake*);  // aktualizacja stanu gry
void game_display(Snake*);      // wyswietlanie gry
int game(int&);                 // petla glowna gry