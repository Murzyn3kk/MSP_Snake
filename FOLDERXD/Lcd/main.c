#include <msp430x14x.h>
#include "portyLcd.h"
#include "lcd.h"
//#include <cstdlib>      // srand(), rand()
//#include <ctime>        // time()

// ---- Przyciski ---------------------------------------------
#define BUTTON_1                !(P4IN&0x10)
#define BUTTON_2                !(P4IN&0x20)
#define BUTTON_3                !(P4IN&0x40)
#define BUTTON_4                !(P4IN&0x80)

// ---- Nazwy zdefiniowanych znakow ---------------------------
#define SNAKE_UP                0
#define SNAKE_DOWN              1
#define SNAKE_BOTH              2
#define APPLE_UP                3
#define APPLE_DOWN              4
#define SNAKE_UP_APPLE_DOWN     5
#define SNAKE_DOWN_APPLE_UP     6
#define BLANK                   32

// ---- Predkosci gry -----------------------------------------
#define GAME_SPEED_MENU         25
#define GAME_SPEED_1            50
#define GAME_SPEED_2            25
#define GAME_SPEED_3            10

// ---- Definicje pomocnicze ----------------------------------
#define LINE_1                  0x80
#define LINE_2                  0xc0
#define NULL                    0

// ---- Zmienne globalne --------------------------------------
int input;
int counter;
int game_speed;
int bufor[16][4];
int ktory_przycisk = 0;

// ---- Deklaracje zmiennych ----------------------------------
struct Snake_segment
{
  Snake_segment(int xa, int ya)
    : x(xa), y(ya) { next = NULL; }

  int x;
  int y;
  Snake_segment* next;
};

class Snake
{
public:
  Snake();

  void display();
  void move(Snake_segment* current);

  int x_direction;
  int y_direction;
  Snake_segment* head;
};

// ---- Deklaracje funkcji ------------------------------------
void Send_String(const char* string, unsigned char line);
void Send_String_XY(const char* string, int x, int y);

void CreateCustomCharacters();  // utworzenie znakow specjalnych

int menu();                     // petla glowna menu
int menu_update();              // aktualizacja stanu menu
void menu_display();            // wyswietlanie menu

int game();                     // petla glowna gry
int game_update(Snake*);        // aktualizacja stanu gry
void game_display(Snake*);      // wyswietlanie gry

int highscores(int);            // petla glowna wynikow
int highscores_update(int&);    // aktualizacja stanu wynikow
void highscores_display();      // wyswietlanie wynikow

// ---- main() ------------------------------------------------
void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  //srand(time(NULL));
  P4DIR = ~0xf0;                 // przyciski
  InitPortsLcd();
  InitLCD();
  clearDisplay();

  BCSCTL1 |= XTS;               // ACLK = LFXT1 = HF XTAL 8MHz
  do 
  {
    IFG1 &= ~OFIFG;                     // Czyszczenie flgi OSCFault
    for (int i = 0xFF; i > 0; i--);     // odczekanie
  }while ((IFG1 & OFIFG) == OFIFG);     // dopoki OSCFault jest ciagle ustawiona
  
  BCSCTL1 |= DIVA_1;                    // ACLK = 8 MHz/2 = 4MHz
  BCSCTL2 |= SELM0 | SELM1;             // MCLK = LFTX1 = ACLK
  TACTL = TASSEL_1 + MC_1 + ID_3;       // Wybieram ACLK, tryb Up, ACLK/8 = 500kHz
  CCTL0 = CCIE;                         // wlaczenie przerwan od CCR0
  CCR0 = 50000;                         // Przerwanie generowane co 20 ms
  _EINT();                              // Wlaczenie przerwan

  game_speed = GAME_SPEED_MENU;
  CreateCustomCharacters();
  int app_state;

  while(1)
  {
    app_state = menu();
    switch(app_state)
    {
    case 1:
      game();
      break;
    case 2:
      highscores(0);
      break;
    default:
      break;
    }
  }
}

// ---- Funkcje pomocnicze ------------------------------------
void Send_String(const char* string, unsigned char line)
{
  SEND_CMD(line);

  while(*string != '?')
    SEND_CHAR(*string++);
}

void Send_String_XY(const char* string, int x, int y)
{
  if(x == 0)
    SEND_CMD(LINE_1);
  else
    SEND_CMD(LINE_2);

  for(int i = 0; i < y; i++)
    SEND_CMD(CUR_SHIFT_RIGHT);

  while(*string != '?')
    SEND_CHAR(*string++);
}

// ---- Inicjacja aplikacji -----------------------------------
void CreateCustomCharacters()                   // utworzenie znakow specjalnych
{
  char c0[] = {0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00}; // SNAKE_UP
  char c1[] = {0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f, 0x1f}; // SNAKE_DOWN
  char c2[] = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f}; // SNAKE_BOTH
  char c3[] = {0x0e, 0x11, 0x11, 0x0e, 0x00, 0x00, 0x00, 0x00}; // APPLE_UP
  char c4[] = {0x00, 0x00, 0x00, 0x00, 0x0e, 0x11, 0x11, 0x0e}; // APPLE_DOWN
  char c5[] = {0x1f, 0x1f, 0x1f, 0x1f, 0x0e, 0x11, 0x11, 0x0e}; // SNAKE_UP_APPLE_DOWN
  char c6[] = {0x0e, 0x11, 0x11, 0x0e, 0x1f, 0x1f, 0x1f, 0x1f}; // SNAKE_DOWN_APPLE_UP

  SEND_CMD(CG_RAM_ADDR);
  for(int i=0; i<8; i++) SEND_CHAR(c0[i]);
  for(int i=0; i<8; i++) SEND_CHAR(c1[i]);
  for(int i=0; i<8; i++) SEND_CHAR(c2[i]);
  for(int i=0; i<8; i++) SEND_CHAR(c3[i]);
  for(int i=0; i<8; i++) SEND_CHAR(c4[i]);
  for(int i=0; i<8; i++) SEND_CHAR(c5[i]);
  for(int i=0; i<8; i++) SEND_CHAR(c6[i]);

  SEND_CMD(CUR_HOME);
}

// ---- Menu --------------------------------------------------
int menu()                                      // petla glowna menu
{
  game_speed = GAME_SPEED_MENU;
  int loop = 0;

  while(!loop)
  {
    loop = menu_update();
    menu_display();

    //input = 0;
    //_BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
  }

  return loop;
}

int menu_update()                               // aktualizacja stanu menu
{
  if(BUTTON_1)
    return 1;
  if(BUTTON_2)
    return 2;

  return input;
  switch(input)
  {
  case 1:
    return 1;
    break;
  case 2:
    return 2;
    break;
  }

  return 0;
}

void menu_display()                             // wyswietlanie menu
{
  clearDisplay();

  Send_String("Menu?", LINE_1);
}

// ---- Snake -------------------------------------------------
Snake::Snake()
{
  head = new Snake_segment(2, 0);
  head->next = new Snake_segment(1, 0);
  head->next->next = new Snake_segment(0, 0);

  y_direction = 0;
  x_direction = 1;
}

void Snake::display()                // wyswietlanie weza
{
  Snake_segment* current = head;
  while(current)
  {
    bufor[current->x][current->y] = 1;
    current = current->next;
  }
}

void Snake::move(Snake_segment* current)        // poruszanie weza
{
    if(current->next->next != NULL)
        move(current->next);

    current->next->x = current->x;
    current->next->y = current->y;
}

// ---- Gra ---------------------------------------------------
int game()                                      // petla glowna gry
{
  game_speed = GAME_SPEED_1;
  int loop = 0;

  Snake* snake = new Snake();

  input = 0;
  while(!loop)
  {
    loop = game_update(snake);
    game_display(snake);
    while(1)
    {
        if(counter%15 == 0)
        {
                counter = 0;
                break;
        }
    }
    //input = 0;
    //_BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
  }

  // highscores(score);

  return loop;
}

int game_update(Snake* snake)                   // aktualizacja stanu gry
{
  // przetwarzanie inputu
    if(ktory_przycisk == 1)
    if(snake->x_direction == 0)
    {
      snake->x_direction = -1;
      snake->y_direction = 0;
    }
    //break;
  //case 2:       // button 2
    if(ktory_przycisk == 2)
    if(snake->x_direction == 0)
    {
      snake->x_direction = 1;
      snake->y_direction = 0;
    }
    //break;
  //case 3:       // button 3
    if(ktory_przycisk == 3)
    if(snake->y_direction == 0)
    {
      snake->x_direction = 0;
      snake->y_direction = -1;
    }
    //break;
  //case 4:       // button 4
    if(ktory_przycisk == 4)
    if(snake->y_direction == 0)
    {
      snake->x_direction = 0;
      snake->y_direction = 1;
    }
    //break;
  //}

  // sprawdzanie kolizji ze sciana
  int next_head_pos_x = snake->head->x + snake->x_direction;
  int next_head_pos_y = snake->head->y + snake->y_direction;

  if(next_head_pos_x > 15 || next_head_pos_x < 0)
    return 1;
  if(next_head_pos_y > 3 || next_head_pos_y < 0)
    return 1;

  // sprawdzanie kolizji z wezem
  Snake_segment* current = snake->head->next->next->next;
  while(current)
  {
    if(next_head_pos_x == current->x && next_head_pos_y == current->y)
      return 1;
    current = current->next;
  }

  // aktualizacja pozycji weza
  snake->move(snake->head);

  snake->head->x = next_head_pos_x;
  snake->head->y = next_head_pos_y;

  return 0;
}

void game_display(Snake* snake)                 // wyswietlanie gry
{
  clearDisplay();

  //char** bufor = new char*[16];
  for(int i = 0; i < 16; i++)
  {
    //bufor[i] = new char[4];
    for(int j = 0; j < 4; j++)
      bufor[i][j] = 0;
  }

  snake->display();
  // display food

  // konwersja bufora na znaki
  SEND_CMD(LINE_1);
  SEND_CMD(CUR_HOME);
  for(int i = 0; i < 16; i++)
  {
    if(bufor[i][0] == 0 && bufor[i][1] == 0)
      SEND_CHAR(BLANK);
    else if(bufor[i][0] == 1 && bufor[i][1] == 0)
      SEND_CHAR(SNAKE_UP);
    else if(bufor[i][0] == 0 && bufor[i][1] == 1)
      SEND_CHAR(SNAKE_DOWN);
    else if(bufor[i][0] == 1 && bufor[i][1] == 1)
      SEND_CHAR(SNAKE_BOTH);
    else if(bufor[i][0] == 2 && bufor[i][1] == 0)
      SEND_CHAR(APPLE_UP);
    else if(bufor[i][0] == 0 && bufor[i][1] == 2)
      SEND_CHAR(APPLE_DOWN);
    else if(bufor[i][0] == 1 && bufor[i][1] == 2)
      SEND_CHAR(SNAKE_UP_APPLE_DOWN);
    else
      SEND_CHAR(SNAKE_DOWN_APPLE_UP);
  }
  SEND_CMD(LINE_2);
  for(int i = 0; i < 16; i++)
  {
    if(bufor[i][2] == 0 && bufor[i][3] == 0)
      SEND_CHAR(BLANK);
    else if(bufor[i][2] == 1 && bufor[i][3] == 0)
      SEND_CHAR(SNAKE_UP);
    else if(bufor[i][2] == 0 && bufor[i][3] == 1)
      SEND_CHAR(SNAKE_DOWN);
    else if(bufor[i][2] == 1 && bufor[i][3] == 1)
      SEND_CHAR(SNAKE_BOTH);
    else if(bufor[i][2] == 2 && bufor[i][3] == 0)
      SEND_CHAR(APPLE_UP);
    else if(bufor[i][2] == 0 && bufor[i][3] == 2)
      SEND_CHAR(APPLE_DOWN);
    else if(bufor[i][2] == 1 && bufor[i][3] == 2)
      SEND_CHAR(SNAKE_UP_APPLE_DOWN);
    else
      SEND_CHAR(SNAKE_DOWN_APPLE_UP);
  }

  //for(int i = 0; i < 16; i++)
    //delete[] bufor[i];
  //delete[] bufor;
}

// ---- Wyniki ------------------------------------------------
int highscores(int score)                       // petla glowna wynikow
{
  game_speed = GAME_SPEED_MENU;
  int loop = 0;

  while(!loop)
  {
    loop = highscores_update(score);
    highscores_display();

    input = 0;
    _BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
  }

  return loop;
}

int highscores_update(int& score)               // aktualizacja stanu wynikow
{

  return 0;
}

void highscores_display()                       // wyswietlanie wynikow
{
  clearDisplay();
}

// ---- Timer -------------------------------------------------
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A(void)
{
  if((BUTTON_1)==1)
    ktory_przycisk = 1;
  else if((BUTTON_2)==1)
    ktory_przycisk = 2;
   else if((BUTTON_3)==1)
    ktory_przycisk = 3;
   else if((BUTTON_4)==1)
    ktory_przycisk = 4;
   else
     ktory_przycisk = 404;
  ++counter;
  _BIC_SR_IRQ(LPM3_bits); // wyjscie z trybu oszczednego
}
