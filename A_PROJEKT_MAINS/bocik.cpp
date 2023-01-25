


int time elapsed;
int input;

int main()
{
    BCSCTL1 |= XTS;               
  do
  {
    IFG1 &= ~OFIFG;                     
    for (int i = 0xFF; i > 0; i--);     
  }while ((IFG1 & OFIFG) == OFIFG);     
  BCSCTL1 |= DIVA_1;                    
  BCSCTL2 |= SELM0 | SELM1;             
  TACTL = TASSEL_1 + MC_1 + ID_3;       
  CCTL0 = CCIE;                         
  CCR0 = 10000;                         
  _EINT();      

  input = 0;
  time_elapsed = TAR;                       

    while(1)
    {
        // change direction of snake based on input variable

        // do game logic

        // do the drawing

        input = 0;
        _BIS_SR(LPM3_bits);          
    }
}

#pragma vector=TIMER0_A0_VECTOR
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

    time_elapsed = TAR;

    if(time_elapsed >= GAME_SPEED)
    {
        time_elapsed = 0;
        TACTL |= TACLR;
        _BIC_SR_IRQ(LPM3_bits); 
    }
}

/*
For example, if you are using the ACLK as the clock source with a divider of 8 
and the CCR0 register is set to 10000, the period of the TimerA interrupt is:
4.096kHz / 8 = 512Hz
1 / 512Hz = 1.953125ms








*/