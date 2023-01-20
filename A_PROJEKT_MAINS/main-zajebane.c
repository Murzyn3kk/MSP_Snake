#include <msp430x14x.h> 
#include "lcd.h" 
#include "portyLcd.h" 
#include <time.h> 
#include <stdbool.h> 
#include <math.h> 
#include <cstdlib>
 
//---------------- zmienne globalne ------------- 
unsigned int i=0; 
unsigned int sekundy=0; 
unsigned int licznik=0; 
unsigned int counter=0; 
unsigned int counterFrame = 0; 
unsigned int jumpTime = 0; 
unsigned int jumpButtonPressed = 0; 
unsigned int dropButtonPressed = 0; 
unsigned int notInitialized = 1; 
unsigned int notLost = 1; 
volatile char pixels[2][16] = {0}; 
char character = 'o'; 
char characterTop = 'd'; 
char characterBot = 'z'; 
char cactus = 'H'; 
char bird = '^'; 
int line = 1; 
bool menu = true, score = false, game = false, discr = false, credits = false, gameEnded = false; 
unsigned int highScore = 0; 
unsigned int bestPlayerScore[] = {0,0,0}; 
char bestPlayerName[3][5]; 
 
unsigned int currScore = 0;
unsigned int ScoreMultiplier = 1; 
 
unsigned int przedzial = 0;
unsigned int DifficultyStep = 5;
unsigned int DifficultyStep2 = 15;
unsigned int InBetween = 0; 
 
char abovus[] =         {0x0e, 0x1f, 0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x1b}; //0
char cactusus[] =       {0x04, 0x0A, 0x0B, 0x0A, 0x1A, 0x0B, 0x0A, 0x0E}; //1
char abovusTop[] =      {0x00, 0x00, 0x00, 0x00, 0x0E, 0x1F, 0x11, 0x11}; //2
char abovusBot[] =      {0x1F, 0x1F, 0x1F, 0x1B, 0x00, 0x00, 0x00, 0x00}; //3
char snake[] =          {0x1F, 0x1F, 0x1F, 0x1f, 0x00, 0x00, 0x10, 0x10}; //4
char menuG[] =          {'G', 'A', 'M', 'E', ' ', ' ', ' ', ' ', ' ', 'O' ,'P', 'I', 'S', ' ', ' ', ' ', '?'}; 
char menuD[] =          {'W', 'Y', 'N', 'I', 'K', 'I', ' ', ' ', ' ', 'A' ,'U', 'T', 'O', 'R', 'Z', 'Y', '?'}; 
char opisG[] =          {'P', 'R', 'Z', 'Y', 'C', 'I', 'S', 'K', ' ', '1' ,' ', 'S', 'K', 'O', 'K', ' ', '?'};  
char opisD[] =          {'P', 'R', 'Z', 'Y', 'C', 'I', 'S', 'K', ' ', '2' ,' ', 'S', 'C', 'H', 'Y', 'L', '?'}; 
char creditsA[] =       {'D', 'a', 'w', 'i', 'd', ' ', 'U', 'g', 'n' ,'i', 'e', 'w', 's', 'k', 'i', '?'}; 
char creditsB[] =       {'O', 'o', 'o', 'o', 'o', 'o', ' ', 'O', 'o', 'o' ,'o', 'o', 'o', 'o', 'o', ' ', '?'}; 
char creditsC[] =       {'O', 'o', 'o', 'o', ' ', 'O', 'o', 'o', 'o', 'o' ,'o', 'o', 'o', ' ', ' ', ' ', '?'}; 
char endWin[] =         {'P', 'O', 'B', 'I', 'L', 'E', 'S', ' ', 'R', 'E' ,'K', 'O', 'R', 'D', '!', ' ', '?'}; 
char endLose[] =        {'P', 'O', 'S', 'Z', 'L', 'O', ' ', 'C', 'I', ' ' ,'G', 'O', 'R', 'Z', 'E', 'J', '?'}; 
char endA[] =           {'S', 'C', 'O', 'R', 'E', ':', ' ', ' ', ' ', ' ' ,' ', ' ', ' ', ' ', ' ', ' ', '?'}; 
int menuCnt = 0; 
 
 
void printDiscr(void);
void printScore(void);
void printMenu(void);
void printCred(void);
void Game(void);
void Input(void);
void InputMenu(void);
void Init(void); 
void Send_String(unsigned char characters[], int wybor);
void CreateCharacters(); 
 
 
 
 
//Wypisz liczbe 
int WyswietlLiczbe(int liczba, int prev) 
{   
  int count = 0; 
  if(liczba == 0 && prev < 10) return 0; 
  else count = WyswietlLiczbe(liczba/10, liczba) + 1; 
  SEND_CHAR(liczba%10 + '0'); 
  return count; 
} 
 
//Wypisz napis domyslny dla rekordu w wynikach 
void printString(char a[])
{   
  if(strlen(a)<4){
    SEND_CHAR('B'); 
    SEND_CHAR('R'); 
    SEND_CHAR('A'); 
    SEND_CHAR('K'); 
  } 
  else {
    for(int i = 0; i<4; i++) 
      SEND_CHAR(a[i]); 
  } 
} 
 
//----------------- main program ------------------- 
void main( void )
{   
  srand( time(NULL));   //Przycisk ustawiony na in 
  P4DIR &= ~BIT4; 
  P4DIR &= ~BIT5; 
  P4DIR &= ~BIT6; 
  P4DIR &= ~BIT7; 
 
  WDTCTL=WDTPW + WDTHOLD;           // Wy??czenie WDT 
  InitPortsLcd();                   // inicjalizacja port?w LCD 
  InitLCD();                        // inicjalizacja LCD 
  clearDisplay();                   // czyszczenie wy?wietlacza       
 
  BCSCTL1 |= XTS;                       // ACLK = LFXT1 = HF XTAL 8MHz   
  do { 
    IFG1 &= ~OFIFG;                     // Czyszczenie flgi OSCFault     
    for (i = 0xFF; i > 0; i--);         // odczekanie 
  } while ((IFG1 & OFIFG) == OFIFG);    // dopoki OSCFault jest ciagle ustawiona    
 
  BCSCTL1 |= DIVA_1;                    // ACLK=8 MHz/2=4 MHz 
  BCSCTL2 |= SELM0 | SELM1;             // MCLK= LFTX1 =ACLK 
  TACTL = TASSEL_1 + MC_1 +ID_3;        // Wybieram ACLK, ACLK/8=500kHz,tryb Up 
  CCTL0 = CCIE;                         // w??czenie przerwa? od CCR0   
  CCR0=3000;                           // podzielnik 3000   
  _EINT();                              // w??czenie przerwa? 
   
  
  
 
//Wygeneruj losowa liczbe ustalajaca trudnosc 
  przedzial = DifficultyStep + (rand()%DifficultyStep2); 
CreateCharacters(); 

SEND_CMD(CUR_HOME); 
  for(int z = 0; z<8; ++z)
  {
    SEND_CHAR(4);
  }
  while(1){}
  
//GameLoop 
  for (;;){ 
    _BIS_SR(LPM3_bits);   
    //Wybór opcji menu dzia³a tylko w menu 
    //Oponienie zeby natychmiastowo nie lapalo inputow 
    if(!game && !score && !credits && !discr && menu){        
      for(int i = 0; i < 100; i++); 
      InputMenu(); 
    } 
    //zabezpieczenie przed b³êdem, który powodowa³, ¿e gra nigdy nie koñczy³a siê     
    if(gameEnded && (P4IN & BIT4)!=0) 
      gameEnded = false; 
     
    if(menu){       
      printMenu(); 
      //zabezpieczenie przed b³êdem, który sprawia³, ¿e napis mruga³ zbyt szybko       
      menuCnt++;       
      if(menuCnt == 0) 
      clearDisplay();       
      menuCnt = menuCnt%2; 
    }else if(game && !gameEnded){//Przycisk 1 
      //Przygotuj ekran 
      if(notInitialized==1){ 
        Init(); 
        pixels[0][0] = 0x20;         
        pixels[1][15] = 0x20; 
      } else Game(); 
       
    }else if(discr){//Wyswietl opis 
      printDiscr();   
      menu = true, game = false, score = false, credits = false, discr = false;       
      clearDisplay(); 
    }else if(score){//Wyswietl wynik 
      printScore(); 
      menu = true, game = false, score = false, credits = false, discr = false;       
      for(int i = 0; i < 100; i++)       
        clearDisplay(); 
    }else if(credits){//Wyswietl tworcow 
      printCred();   
      menu = true, game = false, score = false, credits = false, discr = false;       
      clearDisplay(); 
    }  
  } 
} 
 
//----------------- wypisanie napisu ------------------- //wypisuje napis(tablice znakow) w wybranej przez nas linii  
void Send_String(unsigned char characters[], int wybor)
{ 
  if(wybor == 1) 
    SEND_CMD(DD_RAM_ADDR); 
  else if(wybor == 2) 
    SEND_CMD(DD_RAM_ADDR2);   
  else 
    return;
  
  while(*characters != '?') 
    SEND_CHAR(*characters++); 
} 
 
//----------------- utworzenie specjalnych znaków ------------------- 
void CreateCharacters()
{ 
  SEND_CMD(CG_RAM_ADDR);
  for(int i=0; i<8; i++) SEND_CHAR(abovus[i]);
  for(int i=0; i<8; i++) SEND_CHAR(cactusus[i]);
  for(int i=0; i<8; i++) SEND_CHAR(abovusTop[i]);
  for(int i=0; i<8; i++) SEND_CHAR(abovusBot[i]);
  for(int i=0; i<8; i++) SEND_CHAR(snake[i]);
  
  SEND_CMD(CUR_HOME); 
} 
 
//----------------- wypisanie menu ------------------- 
void printDiscr()
{  
  Send_String(opisG, 1);  
  Send_String(opisD, 2); 
  for(int i = 0; i < 200; i++)
    Delayx100us(700); 
} 

void printMenu()
{  
  Send_String(menuG, 1); 
  Send_String(menuD, 2); 
} 

void printCred()
{   
  Send_String(creditsA, 1);
  Send_String(creditsB, 2);
  for(int i = 0; i < 50; i++)
    Delayx100us(700);
  clearDisplay();
  
  Send_String(creditsC, 1);
  for(int i = 0; i < 50; i++)
    Delayx100us(700);
}

void printScore()
{ 
  clearDisplay(); 
  SEND_CMD(DD_RAM_ADDR); 
  
  //Wyswietl nazwe w pierwszej linii     
  printString(bestPlayerName[0]); 
  SEND_CHAR(' ');
  
  //Wyswietl wynik pierwszego gracza 
  WyswietlLiczbe(bestPlayerScore[0],10); 
  SEND_CMD(DD_RAM_ADDR2); 
  
  //Wyswietl wynik i nazwe drugiego gracza     
  printString(bestPlayerName[1]); 
  SEND_CHAR(' '); 
  WyswietlLiczbe(bestPlayerScore[1],10); 
     
  //Odczekaj chwile 
  for(int i = 0; i < 50; i++)
    Delayx100us(700); 
  clearDisplay(); 
  SEND_CMD(DD_RAM_ADDR); 
  
  //Wyswietl ostatniego - trzeciego gracza  
  printString(bestPlayerName[2]);   
  SEND_CHAR(' '); 
  int LenOfP3 = WyswietlLiczbe(bestPlayerScore[2],10); 
  for(int i = 0; i < 50; i++) 
    Delayx100us(700); 
} 
 
//----------------- zainicjalizowanie ekranu gry ------------------- 
void Init(void) 
{   
  counter++; 
  for(;counter == 32;counter++)
  {
    pixels[counter/16][counter%16] = 0x20;
  }
  counter = counter % 31; //mod(31) poniewa¿ sa 32 dostepne pola na plytce
     if(counter==0) notInitialized = 0;  
} 

//----------------- przyciski w menu ------------------- 
void InputMenu(void)
{ 
  if((P4IN & BIT4)==0 && !gameEnded){ 
    game = true; 
    menu = false; 
  } 
  if((P4IN & BIT5)==0){
    discr = true;
    menu = false; 
  } 
  if((P4IN & BIT6)==0){
    score = true; 
    menu = false; 
  } 
  if((P4IN & BIT7)==0){
    credits = true;
    menu = false; 
  } 
} 
 
//----------------- przyciski w grze ------------------- 
void Input(void)
{ 
  //Jezeli przycisk wcisniety, ustaw linie rysowania naszego bohatera na g?rn? 
  //I ustaw timer po kt?rym spadnie na 4 sekundy 
  if(jumpButtonPressed==1 && jumpTime == 0){
    line=0;
    jumpTime=5;
    InBetween = 2;
  }
  
  //Jezeli wcisniety przycisk kucania, wyladuj na ziemi 
  if(dropButtonPressed==1){ 
    line=1; 
    jumpTime=0; 
  } 
  
  //Mija jedna sekunda skakania 
  if(jumpTime>0) 
    jumpTime--;
  if(jumpTime<=0){ 
    line=1; 
    jumpButtonPressed = 0;
    dropButtonPressed = 0; 
    //Jak wystarczaj?co d?ugo b?dzie na g?rze to zmu? go do updaku i "odkliknij" przycisk 
  } 
} 
 
//----------------- sama gra ------------------- 
void Game(void){ 
  //Je?eli przyciski wci?ni?ty to wrzu? do globalnej info o tym dla nastepnych tikow zegara   
  if((P4IN & BIT4)==0) jumpButtonPressed = 1;   
  if((P4IN & BIT5)==0) dropButtonPressed = 1; 
  if(dropButtonPressed == 1 && jumpButtonPressed == 1) jumpButtonPressed = 0; 
   
  if(counter==0){ 
    //Counter jest modulo 32 wiec co 32 tiki zegra wszystko sie narysuje i zlapiemy input     
    Input(); 
    //Co 15/20 narysowac ekranu zwiekszamy score multiplier ktory wyznacza jak     //duzo punktow dostajemy oraz zwiekszamy szanse na postawienie przeciwnika     
    currScore += ScoreMultiplier; 
    counterFrame++; 
    if(counterFrame==15){ 
      if(DifficultyStep > 1) {         DifficultyStep -= 1; 
        ScoreMultiplier++; 
      } 
      else if(DifficultyStep2 > 2) { 
        DifficultyStep2 -= 1; 
        ScoreMultiplier+=2; 
      } 
    } 
    counterFrame = counterFrame % 20;     if(counterFrame == przedzial) {         //szansa na postawienie przeciwnika 
        przedzial = DifficultyStep + (rand()%DifficultyStep2); 
         
        if(counterFrame%2==0) pixels[0][15] = bird;                   if(counterFrame%2==1) pixels[1][15] = cactus;      
 
        if(pixels[0][15]==cactus) pixels[0][15] = bird;         if(pixels[1][15]==bird) pixels[1][15] = cactus; 
         
        counterFrame = 0; 
    } 
    //Ustaw cursor na pierwsz? linie lokalizacj? 
    SEND_CMD(CUR_HOME); 
    SEND_CMD(DD_RAM_ADDR); 
  } else if(counter==16){ 
    //Je?eli jeste? w 15 elemencie(pocz?tek drugiej linii) to ustaw kursor 
    //na pocz?tek linii drugiej 
    SEND_CMD(CUR_HOME); 
    SEND_CMD(DD_RAM_ADDR2); 
}
//Nie nadpisuj animacji 
if(counter!=31 && counter!=15  
   && pixels[counter/16][(counter%16)+1]!=character 
   && pixels[counter/16][(counter%16)+1]!=characterTop  
   && pixels[counter/16][(counter%16)+1]!=characterBot){ 
   
  pixels[counter/16][counter%16] = pixels[counter/16][(counter%16)+1]; 
    pixels[counter/16][(counter%16)+1] = 0x20; 
  } 
   
  //Tworzenie animacji przy skoku   
  if(counter%16==1){     if(InBetween==2){ 
      pixels[0][1] = characterTop;       InBetween -= 1; 
    } else if (InBetween==1) {       pixels[1][1] = characterBot; 
      InBetween -= 1; 
    } else { 
      if(line==0) pixels[0][1] = character;       else pixels[1][1] = character;      
    } 
  } 
  //Wyslij znak, bohatera lub animacje zaleznie od tego co mamy w tablicy   
  if(pixels[counter/16][counter%16] == characterTop) SEND_CHAR(2);    else if(pixels[counter/16][counter%16] == characterBot) SEND_CHAR(3);   else if(pixels[counter/16][counter%16] == character) SEND_CHAR(0);   else if(pixels[counter/16][counter%16] == cactus) SEND_CHAR(1);   else SEND_CHAR(pixels[counter/16][counter%16]); 
 
  counter++; 
  //Mod 32 bo od 0 do 31 jest 2 * 16 co oznacza ilosc pixeli w liniach   counter = counter % 32; 
  //Kolizje 
  if(counter==0){       //Lewy bok kaktusa 
    if(line == 1 && jumpButtonPressed != 1 && pixels[1][2] == cactus) notLost = 0;  
    //Górny bok kaktusa 
    if(dropButtonPressed == 1 && jumpTime > 0 && (pixels[1][2] == cactus)) notLost = 0;     if(line == 0 && jumpTime == 1 && pixels[1][2] ==cactus) notLost = 0; 
    //Lewy bok ptaka 
    if(line == 0 && jumpTime > 1 && pixels[0][2] == bird) notLost = 0; 
    //Dolny bok ptaka 
    if(jumpButtonPressed == 1 && jumpTime <= 0 && (pixels[0][2] == bird)) notLost = 0;    //W momencie w ktorym przegramy musimy zebrac dane uzytkownika i wyswietlic highscore 
    if(notLost == 0){ 
menu = true, game = false, score = false, credits = false, discr = false, gameEnded = true; bool end = false, buttonHold = false, buttonHold1 = false, buttonHold2 = false; int charValue = 65, picked = 0; char name[4] = "AAAA"; 
while(1){ 
  //Zmien pozycje zmienianego znaku 
if((P4IN & BIT6)==0 && !buttonHold){   picked++; 
    charValue = 65;     if(picked==4) break;     buttonHold = true; 
  }  
  if((P4IN & BIT6)!=0){ 
        buttonHold = false; 
        } 
        //Porusz sie w znakach "do gory"         
if((P4IN & BIT4)==0 && !buttonHold1){           charValue++; 
          if(charValue==90) charValue = 65; 
          buttonHold1 = true; 
        } 
        if((P4IN & BIT4)!=0){ 
          buttonHold1 = false; 
        } 
        //Porusz sie w znakach "do doou"         
if((P4IN & BIT5)==0 && !buttonHold2){           charValue--; 
          if(charValue<65) charValue = 90;  
          buttonHold2 = true; 
        }  
        if((P4IN & BIT5)!=0){           buttonHold2 = false; 
        } 
         
        //Rysuj nazwe wprowadzana         
clearDisplay(); 
        SEND_CMD(DD_RAM_ADDR); 
        for(int i=0; i<6;i++) SEND_CHAR(' '); 
        SEND_CHAR(name[0]); 
        SEND_CHAR(name[1]); 
        SEND_CHAR(name[2]); 
        SEND_CHAR(name[3]); 
        SEND_CMD(DD_RAM_ADDR2); 
         
        //Dodaj znacznik ktory pokazuje ktora litere zmieniasz         
        for(int i=0; i<15;i++){ 
          if(i-(picked+6)==0) SEND_CHAR('^'); 
          else SEND_CHAR(' ');    
        } 
        name[picked] = charValue; 
      } 
   
  //Jezeli pobiles highscore nadpisz go oraz wyswietl inny tekst 
if(currScore > highScore) {   highScore = currScore;   clearDisplay();   Send_String(endWin, 1); 
  for(int i = 0; i < 75; i++) Delayx100us(700); 
}else { 
clearDisplay(); 
Send_String(endLose, 1);   for(int i = 0; i < 75; i++) Delayx100us(700); 
} 
 
//Zapisz highscore do tablicy 
for(int i=0; i < 11; i++){ 
      int tenPower = 1; 
        for(int j=0; j < i; j++) tenPower *= 10;         endA[15-i] = (currScore/tenPower)%10 + '0'; 
      } 
       
      //Usun niepotrzebne zera       
for(int i=5; i<16; i++){         int number = endA[i] - 48;         if(number != 0) break; 
        else endA[i] = ' ';  
      } 
      Send_String(endA, 2); 
       
      //Spaghetti odpowiadajace za sortowanie graczy 
      if(currScore > bestPlayerScore[0]){         bestPlayerScore[2] = bestPlayerScore[1]; 
        strcpy(bestPlayerName[2], bestPlayerName[1]);         for(int i = 0; i<4; i++){ 
          bestPlayerName[2][i] = bestPlayerName[1][i]; 
        } 
        bestPlayerScore[1] = bestPlayerScore[0];         for(int i = 0; i<4; i++){ 
          bestPlayerName[1][i] = bestPlayerName[0][i]; 
        } 
        bestPlayerScore[0] = currScore;          strcpy(bestPlayerName[0], name); 
      } else if (currScore == bestPlayerScore[0] && currScore > bestPlayerScore[1]){         bestPlayerScore[2] = bestPlayerScore[1];         for(int i = 0; i<4; i++){ 
          bestPlayerName[2][i] = bestPlayerName[1][i]; 
        } 
        bestPlayerScore[1] = currScore;         strcpy(bestPlayerName[1], name); 
      } else if (currScore == bestPlayerScore[0] && currScore == bestPlayerScore[1]){         if(currScore > bestPlayerScore[2]){           bestPlayerScore[2] = currScore; 
          strcpy(bestPlayerName[2], name); 
        }  
  } else if (currScore > bestPlayerScore[1]) {     bestPlayerScore[2] = bestPlayerScore[1]; 
  for(int i = 0; i<4; i++){ 
    bestPlayerName[2][i] = bestPlayerName[1][i]; 
  }; 
  bestPlayerScore[1] = currScore;    strcpy(bestPlayerName[1], name); } else if (currScore > bestPlayerScore[2]) { bestPlayerScore[2] = currScore;  strcpy(bestPlayerName[2], name); 
} 
 
currScore = 0;  for(int i = 0; i < 75; i++) Delayx100us(700);    
for(int i = 0; i < 100; i++); 
      clearDisplay(); 
       
      //Ustaw wszystkie zmienne na defaultowe/poczatkowe po smierci 
      notLost = 1, jumpTime = 0, DifficultyStep = 5, DifficultyStep2 = 15; ScoreMultiplier = 1;       for(int i = 0; i < 2; i++)         for(int j = 0; j < 16; j++) 
          pixels[i][j] = 0x20;    
    } 
    //Jezeli kaktus w lewym dolnym rogu, usun go     
    if(pixels[1][0] == cactus) pixels[1][0] = 0x20;  
  } 
} 

 
//----------------- procedura obs³ugi przerwania od TimerA ------------------- 
#pragma vector=TIMERA0_VECTOR 
__interrupt void Timer_A (void){ 
  ++licznik; 
 _BIC_SR_IRQ(LPM3_bits);             // wyjœcie z trybu LPM3 
} 