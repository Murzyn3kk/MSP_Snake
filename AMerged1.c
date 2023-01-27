#include <msp430x14x.h>
#include "portyLcd.h"
#include "lcd.h"
#include <cstdlib>      // srand(), rand()
#include <ctime>        // time()

// ---- Przyciski ---------------------------------------------
#define BUTTON_1                !(P4IN & 0x10)
#define BUTTON_2                !(P4IN & 0x20)
#define BUTTON_3                !(P4IN & 0x40)
#define BUTTON_4                !(P4IN & 0x80)
#define BIT(x)                  (1 << x)

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
#define GAME_SPEED_HIGHSCORES   10
#define GAME_SPEED_1            50
#define GAME_SPEED_2            25
#define GAME_SPEED_3            10

// ---- Funkcje LCD -------------------------------------------
#define LINE_1                  0x80
#define LINE_2                  0xc0
#define DISPLAY_CONTROL         0x0c
#define CURSOR_ON               0x02
#define CURSOR_OFF              0x00
#define BLINK_ON                0x01
#define BLINK_OFF               0x00

// SEND_CMD(DISPLAY_CONTROL | CURSOR_ON | BLINK_ON);
// SEND_CMD(DISPLAY_CONTROL | CURSOR_OFF | BLINK_OFF);

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

  int length;
  int x_direction;
  int y_direction;
  Snake_segment* head;
};

struct Highscore
{
  char* name;
  int score;
};

// ---- Zmienne globalne --------------------------------------
int input;
int time_elapsed;
int game_speed;
int bufor[16][4];
int apple_x;
int apple_y;
Highscore highscore[2];

// ---- Deklaracje funkcji ------------------------------------
void Send_String(char*, unsigned char);
void Send_Int4digits(int);

void CreateCustomCharacters();          // utworzenie znakow specjalnych
void ClearHighscores();                 // reset wynikow

int menu();                             // petla glowna menu
int menu_update(int&);                  // aktualizacja stanu menu
void menu_display();                    // wyswietlanie menu
void initial_menu_display();            // wyswietlanie pierwszego menu

int game();                             // petla glowna gry
void game_settings(int&);               // ustawienia gry
int game_update(Snake*, int&, int&);    // aktualizacja stanu gry
void game_display(Snake*);              // wyswietlanie gry

int highscores(int);                    // petla glowna wynikow
void edit_highscore(int);               // edycja nazwy wyniku
void highscores_add(int&);              // dodanie nowego wyniku
int highscores_update(int&);            // aktualizacja stanu wynikow
void highscores_display();              // wyswietlanie wynikow

void show_authors();                    // wyswietlanie autorow
void show_control();                    // wyswietlanie sterowania

// ---- main() ------------------------------------------------
void main()
{
  WDTCTL = WDTPW + WDTHOLD;
  srand(time(NULL));
  P4DIR = ~0xf0;                // przyciski
  InitPortsLcd();
  InitLCD();
  clearDisplay();

  BCSCTL1 |= XTS;                       // ACLK = LFXT1 = HF XTAL 8MHz
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
  ClearHighscores();
  int app_state;
  
  initial_menu_display();
  while(input == 0)
    _BIS_SR(LPM3_bits);
  
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
void Send_String(char* string, unsigned char line)
{
  SEND_CMD(line);

  while(*string != '?')
    SEND_CHAR(*string++);
}

void Send_Int4digits(int value)
{
  int position;
  char chars[4] = {0};
  if(value > 999)
    position = 0;
  else if(value > 99)
    position = 1;
  else if(value > 9)
    position = 2;
  else
    position = 3;

  for(int i = 0; i < 4; ++i)
    if(i < position)
      chars[i] = 32;
    else
    {
      chars[i] = value % 10;
      value = value / 10;
    }

  for(int i = 4; i < 0; --i)
    SEND_CHAR(chars[i]);
}

// ---- Inicjacja aplikacji -----------------------------------
void CreateCustomCharacters()                           // utworzenie znakow specjalnych
{
  char c0[] = {0x0e, 0x1f, 0x1f, 0x0e,  0x00, 0x00, 0x00, 0x00}; // SNAKE_UP
  char c1[] = {0x00, 0x00, 0x00, 0x00,  0x0e, 0x1f, 0x1f, 0x0e}; // SNAKE_DOWN
  char c2[] = {0x0e, 0x1f, 0x1f, 0x0e,  0x0e, 0x1f, 0x1f, 0x0e}; // SNAKE_BOTH
  char c3[] = {0x04, 0x0a, 0x0a, 0x04,  0x00, 0x00, 0x00, 0x00}; // APPLE_UP
  char c4[] = {0x00, 0x00, 0x00, 0x00,  0x04, 0x0a, 0x0a, 0x04}; // APPLE_DOWN
  char c5[] = {0x0e, 0x1f, 0x1f, 0x0e,  0x04, 0x0a, 0x0a, 0x04}; // SNAKE_UP_APPLE_DOWN
  char c6[] = {0x04, 0x0a, 0x0a, 0x04,  0x0e, 0x1f, 0x1f, 0x0e}; // SNAKE_DOWN_APPLE_UP

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

void ClearHighscores()                                  // reset wynikow
{
  highscore[0].name = new char[9];
  highscore[1].name = new char[9];
  for(int i = 0; i < 8; i++)
  {
    highscore[0].name[i] = 65;  // set to 32
    highscore[1].name[i] = 66;
  }
  highscore[0].name[8] = '?';
  highscore[1].name[8] = '?';
  highscore[0].score = 1234;    // set to 0
  highscore[1].score = 7;
}

// -----------------------------------------------------------------------------
// ---- Menu -------------------------------------------------------------------
// -----------------------------------------------------------------------------
int menu()                                              // petla glowna menu
{
  game_speed = GAME_SPEED_MENU;
  int loop = 0;
  int display_position = 0;

  menu_display();
  while(!loop)
  {
    input = 0;
    _BIS_SR(LPM3_bits);
    loop = menu_update(display_position);
  }

  return loop;
}

int menu_update(int& display_position)                  // aktualizacja stanu menu
{
  if(input)
    return input;
  
  if(++display_position == 10)
  {
    display_position = 0;
    menu_display();
  }
  else
    SEND_CMD(DATA_ROL_LEFT); //przewijanie tekstu w prawo

  return 0;
}

void menu_display()                                     // wyswietlanie menu
{
  clearDisplay();

  Send_String("1 - Graj       2 - Wyniki?", LINE_1);
  Send_String("3 - Sterowanie 4 - Autorzy?", LINE_2);
}

void initial_menu_display()                             // wyswietlanie poczatkowego menu
{
  clearDisplay();
  Send_String("      Snake?", LINE_1);
  Send_String("Wcisnij 1?", LINE_2);
}

// -----------------------------------------------------------------------------
// ---- Snake ------------------------------------------------------------------
// -----------------------------------------------------------------------------
Snake::Snake()
{
  head = new Snake_segment(2, 0);
  head->next = new Snake_segment(1, 0);
  head->next->next = new Snake_segment(0, 0);

  length = 3;
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

void Snake::display()                                   // wyswietlanie weza
{
  Snake_segment* current = head;
  while(current)
  {
    bufor[current->x][current->y] = 1;
    current = current->next;
  }
}

void Snake::move(Snake_segment* current)                // poruszanie weza
{
    if(current->next->next != NULL)
        move(current->next);

    current->next->x = current->x;
    current->next->y = current->y;
}

// -----------------------------------------------------------------------------
// ---- Gra --------------------------------------------------------------------
// -----------------------------------------------------------------------------
int game()                                              // petla glowna gry
{
  game_speed = GAME_SPEED_HIGHSCORES;
  int settings = 0x01;
  game_settings(settings);
  
  if(settings & 0x04)
    game_speed = GAME_SPEED_3;
  else if(settings & 0x02)
    game_speed = GAME_SPEED_2;
  else 
    game_speed = GAME_SPEED_1;
  
  int loop = 0;
  int score = 0;

  Snake* snake = new Snake();
  apple_x = 7;
  apple_y = 2;

  while(!loop)
  {
    input = 0;
    _BIS_SR(LPM3_bits);
    loop = game_update(snake, score, settings);
    game_display(snake);
  }

  highscores(score);

  delete snake;

  return loop;
}

void game_settings(int& settings)
{
  while(1)
  {
    input = 0;
    _BIS_SR(LPM3_bits);
    
    if(input == 1)
    {
      if(((settings & 0x03) % 10) == 3)
        settings &= 0xfc;
      else
        settings++;
    }
    if(input == 2)
      settings ^= 0x04;
    if(input == 4)
      break;
    
    clearDisplay();
    Send_String("Predkosc ?", LINE_1);
    SEND_CHAR(((settings & 0x03) % 10) + 48);
    Send_String("Przyspieszenie ?", LINE_2);
    SEND_CHAR((settings & 0x04 ? 1 : 0) + 48);
  }
}

int game_update(Snake* snake, int& score, int& settings)// aktualizacja stanu gry
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
    score = score + ((settings & 0x03) % 10) + ((settings & 0x04) == 0x04 ? 1 : 0);
    if(snake->length % 2 && (settings & 0x04) == 0x04)
      game_speed--;
    
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
    snake->length++;
  }

  // aktualizacja pozycji weza
  snake->move(snake->head);

  snake->head->x = next_head_pos_x;
  snake->head->y = next_head_pos_y;

  return 0;
}

void game_display(Snake* snake)                         // wyswietlanie gry
{
  clearDisplay();

  for(int i = 0; i < 16; i++)
    for(int j = 0; j < 4; j++)
      bufor[i][j] = 0;

  snake->display();
  bufor[apple_x][apple_y] = 2;

  // konwersja bufora na znaki
  SEND_CMD(LINE_1);
  for(int j = 0; j < 4; j+=2)
  {
    if(j == 2) 
      SEND_CMD(LINE_2);
    
    for(int i = 0; i < 16; i++)
      if(bufor[i][j] == 0 && bufor[i][j + 1] == 0)
        SEND_CHAR(BLANK);
      else if(bufor[i][j] == 1 && bufor[i][j + 1] == 0)
        SEND_CHAR(SNAKE_UP);
      else if(bufor[i][j] == 0 && bufor[i][j + 1] == 1)
        SEND_CHAR(SNAKE_DOWN);
      else if(bufor[i][j] == 1 && bufor[i][j + 1] == 1)
        SEND_CHAR(SNAKE_BOTH);
      else if(bufor[i][j] == 2 && bufor[i][j + 1] == 0)
        SEND_CHAR(APPLE_UP);
      else if(bufor[i][j] == 0 && bufor[i][j + 1] == 2)
        SEND_CHAR(APPLE_DOWN);
      else if(bufor[i][j] == 1 && bufor[i][j + 1] == 2)
        SEND_CHAR(SNAKE_UP_APPLE_DOWN);
      else
        SEND_CHAR(SNAKE_DOWN_APPLE_UP);
  }
}

// -----------------------------------------------------------------------------
// ---- Wyniki -----------------------------------------------------------------
// -----------------------------------------------------------------------------
int highscores(int score)                               // petla glowna wynikow
{
  game_speed = GAME_SPEED_HIGHSCORES;
  int loop = 0;

  if(score)
    highscores_add(score);

  highscores_display();
  while(!loop)
  {
    input = 0;
    _BIS_SR(LPM3_bits);
    loop = highscores_update(score);
  }

  return loop;
}

void edit_highscore(int position)                       // edycja nazwy wyniku
{
  int cur_pos = 0;
  while(1)
  {
    input = 0;
    _BIS_SR(LPM3_bits);
    
    switch(input)
    {
      case 1:
        highscore[position].name[cur_pos]--;
        if(highscore[position].name[cur_pos] == 64)
          highscore[position].name[cur_pos] = 32;
        if(highscore[position].name[cur_pos] == 31)
          highscore[position].name[cur_pos] = 90;
        break;
      case 2:
        highscore[position].name[cur_pos]++;
        if(highscore[position].name[cur_pos] == 91)
          highscore[position].name[cur_pos] = 32;
         if(highscore[position].name[cur_pos] == 33)
          highscore[position].name[cur_pos] = 65;
        break;
      case 3:
        cur_pos--;
        if(cur_pos == -1)
          return;
        break;
      case 4:
        cur_pos++;
        if(cur_pos == 8)
          return;
        break;
    }
    
    clearDisplay();
    Send_String("Nowy Rekord!?", LINE_1);
    Send_String(highscore[position].name , LINE_2);
  }
}

void highscores_add(int& score)                         // dodanie nowego wyniku
{
  if(score > highscore[1].score)
  {
    if(score > highscore[0].score)
    {
      highscore[1].score = highscore[0].score;
      highscore[1].name = highscore[0].name;
      highscore[0].score = score;
      edit_highscore(0);
    }
    else
    {
      highscore[1].score = score;
      edit_highscore(1);
    }
  }
}

int highscores_update(int& score)                       // aktualizacja stanu wynikow
{
  if(input)
    return input;

  return 0;
}

void highscores_display()                               // wyswietlanie wynikow
{
  clearDisplay();

  Send_String(highscore[0].name , LINE_1);
  SEND_CHAR(32);
  Send_Int4digits(highscore[1].score);

  Send_String(highscore[1].name , LINE_2);
  SEND_CHAR(32);
  Send_Int4digits(highscore[1].score);
}

// -----------------------------------------------------------------------------
// ---- Autorzy i Sterowanie ---------------------------------------------------
// -----------------------------------------------------------------------------
void show_authors()                                     // wyswietlanie autorow
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

void show_control()                                     // wyswietlanie sterowania
{
  clearDisplay();
  Send_String("1:Lewo 2:Prawo?", LINE_1);
  Send_String("3:Gora 4:Dol?", LINE_2);
  while(input != 1 && input != 2 && input!= 3 && input != 4)
  {}
    //for(long int j=0; j<100000; ++j)
    //{}
}

// -----------------------------------------------------------------------------
// ---- Timer ------------------------------------------------------------------
// -----------------------------------------------------------------------------
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

  if(++time_elapsed >= game_speed)
  {
    time_elapsed = 0;
    _BIC_SR_IRQ(LPM3_bits); // wyjscie z trybu oszczednego
  }
}
