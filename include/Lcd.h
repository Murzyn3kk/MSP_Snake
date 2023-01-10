#pragma once
#include "portyLcd.h"

void InitLCD(void);
void clearDisplay();
void SEND_CHAR (unsigned char c);
void SEND_CMD (unsigned char e);
void Delayx100us(unsigned char b);