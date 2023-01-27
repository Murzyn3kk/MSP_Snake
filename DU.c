#include <msp430x14x.h>
#include "portyLcd.h"
#include "lcd.h"
#include <cstdlib>      // srand(), rand()
#include <ctime>        // time()

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
//#define NULL                    0

// ---- Zmienne globalne --------------------------------------
int input;
int time_elapsed;
int game_speed;
int bufor[16][4];
int apple_x;
int apple_y;

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
  ~Snake();

  void display();
  void move(Snake_segment* current);

  int x_direction;
  int y_direction;
  Snake_segment* head;
};

// ---- Deklaracje funkcji ------------------------------------
void Send_String(const char* string, unsigned char line);
void Send_String_XY(const char* string, int x, int y);
void Send_Int_As_String(int);

void CreateCustomCharacters();  // utworzenie znakow specjalnych

int menu();                     // petla glowna menu
int menu_update();              // aktualizacja stanu menu
void menu_display();            // wyswietlanie menu
void initial_menu_display();    // wyswietlanie pierwszego menu

int game();                     // petla glowna gry
int game_update(Snake*);        // aktualizacja stanu gry
void game_display(Snake*);      // wyswietlanie gry


int highscores(int);            // petla glowna wynikow
int highscores_update(int&);    // aktualizacja stanu wynikow
void highscores_display();      // wyswietlanie wynikow

void show_authors();
void show_control();

// ---- main() ------------------------------------------------
void main()
{

  WDTCTL = WDTPW + WDTHOLD;
  srand(time(NULL));
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
  CCR0 = 10000;                         // Przerwanie generowane co 20 ms
  _EINT();                              // Wlaczenie przerwan
  

  game_speed = GAME_SPEED_MENU;
  CreateCustomCharacters();
  int app_state;
  
  initial_menu_display();
  while(input == 0)
    _BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
  
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
    case 3:
      show_control();
      break;
    case 4:
      show_authors();
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

void Send_Int_As_String(int value)
{
  char liczba[] = {' ', ' ', ' ', ' '};
  int ile_cyfr;
  if (value > 
  
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

  menu_display();
  _BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
  input = 0;
  while(!loop)
  {
    loop = menu_update();
    

    input = 0;
    _BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
  }

  return loop;
}

int menu_update()                               // aktualizacja stanu menu
{
  
  if(input)
    return input;
    
  SEND_CMD(DATA_ROL_LEFT); //przewijanie tekstu w prawo
    
  return 0;
}
void initial_menu_display()
{
  clearDisplay();
  Send_String("      Snake?", LINE_1);
  Send_String("Wcisnij 1?", LINE_2);
}
void menu_display()                             // wyswietlanie menu
{
  clearDisplay();

  Send_String("1 - Graj 2 - Highscores?", LINE_1);
  Send_String("3 - Sterowanie 4 - Autorzy?", LINE_2);
  
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

Snake::~Snake()
{
  Snake_segment* current = head;
  while(current->next)
  {
    Snake_segment* tmp = current->next;
    delete current;
    current = tmp;
  }
  delete current;
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
  apple_x = 7;
  apple_y = 2;

  input = 0;
  while(!loop)
  {
    loop = game_update(snake);
    game_display(snake);
    
    //Delayx100us(200);

    input = 0;
    _BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
  }

  // highscores(score);

  delete snake;

  return loop;
}

int game_update(Snake* snake)                   // aktualizacja stanu gry
{
  // przetwarzanie inputu
  switch(input)
  {
  case 1:       // button 1
    
    if(snake->x_direction == 0)
    {
      snake->x_direction = -1;
      snake->y_direction = 0;
    }
    break;
  case 2:       // button 2
    if(snake->x_direction == 0)
    {
      snake->x_direction = 1;
      snake->y_direction = 0;
    }
    break;
  case 3:       // button 3
    if(snake->y_direction == 0)
    {
      snake->x_direction = 0;
      snake->y_direction = -1;
    }
    break;
  case 4:       // button 4
    if(snake->y_direction == 0)
    {
      snake->x_direction = 0;
      snake->y_direction = 1;
    }
    break;
  }

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

  // sprawdzanie kolizji z jedzeniem
  if(next_head_pos_x == apple_x && next_head_pos_y == apple_y)
  {
    // generowanie nowego jablka
    while(1)
    {
      int new_apple_x = rand() % 16;
      int new_apple_y = rand() % 4;

      Snake_segment* current = snake->head;
      while(current)
      {
        if(next_head_pos_x == new_apple_x && next_head_pos_y == new_apple_y)
          break;
        if(new_apple_x == current->x && new_apple_y == current->y)
          break;
        current = current->next;
      }
      if(current == NULL)
      {
        apple_x = new_apple_x;
        apple_y = new_apple_y;
        break;
      }
    }

    // dodanie segmentu do weza
    Snake_segment* current = snake->head;
    while(current->next)
      current = current->next;
    current->next = new Snake_segment(0,0);
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
  bufor[apple_x][apple_y] = 2;

  // konwersja bufora na znaki
  SEND_CMD(CUR_HOME);
  //SEND_CMD(LINE_1);
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

// ---- Autorzy ------------------------------------------------
void show_authors()
{
  clearDisplay();
  Send_String("Michal Szypluk Dawid Ugniewski?", LINE_1);
  Send_String("Albert Stefanowski Ola Didluch?", LINE_2);
  for(int i = 0; i < 58; ++i)
  {
    SEND_CMD(DATA_ROL_LEFT); //przewijanie tekstu w prawo
    _BIS_SR(LPM3_bits);          // wejscie w tryb oszczedny
    if(input == 1 || input == 2 || input== 3 || input == 4)
      break;
  }
}
// ---- Sterowanie ------------------------------------------------
void show_control()
{
  clearDisplay();
  Send_String("1:Lewo 2:Prawo?", LINE_1);
  Send_String("3:Gora 4:Dol?", LINE_2);
  while(input != 1 && input != 2 && input!= 3 && input != 4)
  {}
    //for(long int j=0; j<100000; ++j)
    //{}
}

// ---- Timer -------------------------------------------------
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A(void)
{
  if (BUTTON_1)
  {
    input = 1;
  }
else if (BUTTON_2)
    input = 2;
  else if (BUTTON_3)
    input = 3;
  else if (BUTTON_4)
    input = 4;

  if(++time_elapsed >= game_speed)
  {
    time_elapsed = 0;
    _BIC_SR_IRQ(LPM3_bits); // wyjscie z trybu oszczednego
  }
}
