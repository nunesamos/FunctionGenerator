#include <msp430.h> 


#define BTN BIT3
#define LCD_OUT P3OUT
#define LCD_DIR P3DIR
#define D4 BIT0
#define D5 BIT1
#define D6 BIT2
#define D7 BIT3
#define RS BIT4
#define E  BIT5
#define DADOS 1
#define COMANDO 0
#define CMND_DLY 800
#define DATA_DLY 800
#define BIG_DLY  10000
#define CLR_DISPLAY  Send_Byte(1, COMANDO, BIG_DLY)
#define POS0_DISPLAY Send_Byte(2, COMANDO, BIG_DLY)


#define SEG_A   BIT0
#define SEG_B   BIT1
#define SEG_C   BIT2
#define SEG_D   BIT3
#define SEG_E   BIT4
#define SEG_F   BIT5
#define SEG_G   BIT6
#define SEG_DP  BIT7



#define S0  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F)
#define S1  (SEG_B + SEG_C)
#define S2  (SEG_A + SEG_B + SEG_D + SEG_E + SEG_G)
#define S3  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_G)
#define S4  (SEG_B + SEG_C + SEG_F + SEG_G)
#define S5  (SEG_A + SEG_C + SEG_D + SEG_F + SEG_G)
#define S6  (SEG_A + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G)
#define S7  (SEG_A + SEG_B + SEG_C)
#define S8  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G)
#define S9  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_F + SEG_G)


const unsigned int digits[10] = {S0, S1, S2, S3, S4, S5, S6, S7, S8, S9};


////////

void Atraso_us(volatile unsigned int us);
void Send_Nibble(volatile unsigned char nibble, volatile unsigned char dados, volatile unsigned int microsegs);
void Send_Byte(volatile unsigned char byte, volatile unsigned char dados, volatile unsigned int microsegs);
void Send_Data(volatile unsigned char byte);
void Send_String(char str[]);
void Send_Int(int n);
void InitLCD(void);


////////

short int k;
unsigned short int duty=0;
unsigned short int frec=10000,amp=100;

unsigned int mem=0;
unsigned int range=0;

//Vetores de sáida

unsigned short int sin[64] = {140, 152, 165, 176, 188, 198, 208, 218, 226, 234, 240, 245, 250, 253, 254, 
                        255, 254, 253, 250, 245, 240, 234, 226, 218, 208, 198, 188, 176, 165, 152, 
                        140, 128, 115, 103, 90, 79, 67, 57, 47, 37, 29, 21, 15, 10, 5, 2, 1, 0, 1, 
                        2, 5, 10, 15, 21, 29, 37, 47, 57, 67, 79, 90, 103, 115, 127, } ;


unsigned short int tri[64] = {8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 135, 143, 151, 159,
                              167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247, 255, 247, 239, 231, 223, 215,
                              207, 199, 191, 183, 175, 167, 159, 151, 143, 135, 128, 120, 112, 104, 96, 88, 80, 72,
                              64, 56, 48, 40, 32, 24, 16, 8, 0};

unsigned short int ser[64] = {8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 135, 143, 151, 159,
                              167, 175, 183, 191, 199, 207, 215, 223, 231, 239, 247, 255, 8, 16, 24, 32, 40, 48, 56,
                              64, 72, 80, 88, 96, 104, 112, 120, 128, 135, 143, 151, 159, 167, 175, 183, 191, 199, 207,
                              215, 223, 231, 239, 247, 255};

unsigned short int sqr[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                              255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, };


//Variáveis de Controle do sistema

unsigned short int Forma_Onda = 1;

unsigned short int indice=0;

unsigned short int Output=0;

volatile unsigned short int i = 0;

int main(void)
{


    WDTCTL = WDTHOLD + WDTPW;

    //////////////////////
    
    //Configuração do DCO para 12Mhz 
    
    UCSCTL3 |= SELREF_2;                      

    UCSCTL4 |= SELA_2;                        


    __bis_SR_register(SCG0);                  

    UCSCTL0 = 0x0000;                         

    UCSCTL1 = DCORSEL_5;                     

    UCSCTL2 = FLLD_1 + 374;                   


    __bic_SR_register(SCG0);                  







    __delay_cycles(375000);//


    do

    {

    UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);

                                   // Clear XT2,XT1,DCO fault flags

    SFRIFG1 &= ~OFIFG;                      // Clear fault flags

    }while (SFRIFG1&OFIFG);


    ////////////////////////
    
    //Configuração dos pinos de saída para o Display 7seg

    P6DIR |= (BIT3 + BIT4 + BIT6);
    P6OUT &= ~(BIT3 + BIT4 + BIT6);

    P8DIR |= (BIT2);
    P8OUT &= ~(BIT2);

    P2DIR |= (BIT2 + BIT3 + BIT4);
    P2OUT &= ~(BIT2 + BIT3 + BIT4);

    ///////////////////////
    
    // Configuração do ADC12

    P6SEL |= BIT2 | BIT1;       // Habilita os canais de entrada analogica

    REFCTL0 &= ~REFMSTR; // Reseta o REFMSTR. Valor de referencia dos potenciometros

    ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_0 + ADC12REFON + ADC12REF2_5V; // Ativacao do conversor analogico-digital.

    ADC12IE   = ADC12IE1 | ADC12IE0 ;      // Habilita as inerrupcoes dos ADC's

    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;//Multiplos canais e habilita a amostragem

    ADC12CTL2 = ADC12RES_0;              // Resolucao de 8 Bits

    ADC12MCTL0 = ADC12SREF_1 | ADC12INCH_2; //Utiliza a referencia de 2,5v de entrada para o primeiro potenciometro

    ADC12MCTL1 = ADC12SREF_1 | ADC12INCH_1; // Referencia de 2,5v para o segundo, seleciona o canal 6.1 de entrada.

    for ( i=0; i<0x30; i++);             // Delay para inicializacao da referencia

    ADC12CTL0 |= (ADC12ENC + ADC12SC);     // Habilita as conversoes ADC.

    ////////////////////

    // Realização da configuração incial do Display LCD
    
    InitLCD();

    ////////////////////
    
    // Configuração dos Timers A0 e A1 no modo de comparação
    
    TA0CCR0 = 700; // Valor inicial do Timer, usando o SMCLK (Modifica a frequencia)
    TA0CCTL0 = CCIE;
    TA0CTL = MC_1 | TASSEL_2;   // Conta no modo UP e seleciona o SMCLK

    TA1CCTL0 = CCIE;
    TA1CCR0 = 20000;    // Valor inicial do Timer para divisao do SMCLK (Modifica a frequencia)
    TA1CCR1 = 10000;
    TA1CCTL1 = CCIE;

    TA1CTL = MC_1 | TASSEL_1 | TACLR ;  // Conta no modo UP e seleciona o ACLK


    // Configuração dos pinos de saída para o DAC

    P1DIR = BIT5 | BIT4 | BIT3 | BIT2;
    P4DIR = 0x0F;
    
    // Configuracao do botao P2.1

    P2DIR&=~BIT1;
    P2REN|=BIT1; //Ativa o resistor de pull/up

    P2OUT|=BIT1; //Seta como resistor de pull/up
    P2IE |=BIT1; //Ativa a interrupção para o botao
    P2IES|=BIT1; //Interrupção na borda de descida

    __enable_interrupt();    // Habilita as interrupcoes

    Send_String("Inicio");
                 
    while(1)
    {
        LPM0;
    }

}

#pragma vector=ADC12_VECTOR
    __interrupt void OndaQuadrada (void){
        switch (__even_in_range(ADC12IV,8)) // Caracteriza prioridades de interrupcoes.
        {
            case  0: break;                           
            case  2: break;                           
            case  4: break;                           
            case  6:

                    amp = ADC12MEM0;

                    ADC12CTL0 &= ~(ADC12SC + ADC12ENC);

                    break;



            case  8:

                    frec = 700 - 2*ADC12MEM1;
                    //Atribui a variacao do potenciometro para a variavel frec
                    TA0CCR0 = frec;              // Atribui o valor de frec para o Timer 2

                    ADC12CTL0 &= ~(ADC12SC + ADC12ENC);

                    break;


            case 10: break;                           
            case 12: break;                           
            case 14: break;                          
            case 16: break;                           
            case 18: break;                           
            case 20: break;                          
            case 22: break;                           
            case 24: break;                           
            case 26: break;                           
            case 28: break;                           
            case 30: break;                           
            case 32: break;                          
            case 34: break;                           
            
            default: break;
        }
        
    }





#pragma vector=PORT2_VECTOR
    __interrupt void P2_Function()
    {



        __delay_cycles(10000);
        
        if((P2IN&BIT1) == 0 ){
            
            __delay_cycles(10000);
            
            if((P2IN&BIT1) == 0 ){
                
                __delay_cycles(10000);
                
                    if((P2IN&BIT1) == 0 ) Forma_Onda++;
                    
            }
            
        }
        
        if (Forma_Onda == 5) Forma_Onda = 1;

        P2IFG&=~BIT1; // Reset Port2 interrupt flag


    }



#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{


    
    if      (Forma_Onda==1) Output = sin[indice]*amp/255;
    else if (Forma_Onda==2) Output = tri[indice]*amp/255;
    else if (Forma_Onda==3) Output = ser[indice]*amp/255;
    else if (Forma_Onda==4) Output = sqr[indice]*amp/255;
    else Output = sin[indice];

    if (indice == 63) {indice = 0;}
    else {indice++;}


    P1OUT = (Output&0xF0)/4;
    P4OUT = (Output&0x0F);

}


#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1 (void)
{



    range++;

    CLR_DISPLAY;
    POS0_DISPLAY;

    Send_String("F:");
    Send_Int(275+1.35*ADC12MEM1);

    Send_String("hz A:");
    Send_Int((0.0256*ADC12MEM0));
    Send_String(".");
    Send_Int((256/100*ADC12MEM0)%10);
    Send_String("V");


}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TMR1()
{
    switch(TA1IV){
    case  0: break;

    case  2:



        P6OUT = (digits[Forma_Onda]&(0x0B))*(0x08);
        P8OUT = (digits[Forma_Onda]&(BIT2));
        P2OUT = (digits[Forma_Onda]&(0x70))/(0x04) + BIT1;

        break;

    case 4:  break;
    case 6:  break;
    case 8:  break;
    case 10: break;                           // Vector 10:  ADC12IFG2
    case 12: break;                           // Vector 12:  ADC12IFG3
    case 14: break;                           // Vector 14:  ADC12IFG4
    default: break;

    }

}

void Atraso_us(volatile unsigned int us)
{
    volatile int id = 0;
    for(id=0;id<(us-1);id++);
}

void Send_Nibble(volatile unsigned char nibble, volatile unsigned char dados, volatile unsigned int microsegs)
{

    LCD_OUT |= E;
    LCD_OUT &= ~(RS + D4 + D5 + D6 + D7);
    LCD_OUT |= RS*(dados==DADOS) +
        D4*((nibble & BIT0)>0) +
        D5*((nibble & BIT1)>0) +
        D6*((nibble & BIT2)>0) +
        D7*((nibble & BIT3)>0);
    LCD_OUT &= ~E;
    Atraso_us(microsegs);

}



void Send_Byte(volatile unsigned char byte, volatile unsigned char dados, volatile unsigned int microsegs)
{

    Send_Nibble(byte >> 4, dados, microsegs/2);
    Send_Nibble(byte & 0xF, dados, microsegs/2);

}



void Send_Data(volatile unsigned char byte)
{

    Send_Byte(byte, DADOS, DATA_DLY);

}



void Send_String(char str[])
{
    while((*str)!='\0')
    {
        Send_Data(*(str++));
    }
}



void Send_Int(int n)
{
    int casa, dig;
    if(n==0)
    {
        Send_Data('0');
        return;
    }
    if(n<0)
    {
        Send_Data('-');
        n = -n;
    }
    for(casa = 10000; casa>n; casa /= 10);
    while(casa>0)
    {
        dig = (n/casa);
        Send_Data(dig+'0');
        n -= dig*casa;
        casa /= 10;
    }
}

void InitLCD(void)
{
    unsigned char CMNDS[] = {0x20, 0x14, 0xC, 0x6};
    unsigned int i;
    Atraso_us(500);
    LCD_DIR |= D4+D5+D6+D7+RS+E;
    Send_Nibble(0x2, COMANDO, CMND_DLY);
    for(i=0; i<4; i++)
        Send_Byte(CMNDS[i], COMANDO, CMND_DLY);
    CLR_DISPLAY;
    POS0_DISPLAY;
}


