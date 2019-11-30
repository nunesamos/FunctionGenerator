#include <msp430f5529.h>

#define pwm_out BIT0

#define SEG_A   BIT0
#define SEG_B   BIT1
#define SEG_C   BIT2
#define SEG_D   BIT3
#define SEG_E   BIT4
#define SEG_F   BIT5
#define SEG_G   BIT6
#define SEG_DP  BIT7

#define D0  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F)
#define D1  (SEG_B + SEG_C)
#define D2  (SEG_A + SEG_B + SEG_D + SEG_E + SEG_G)
#define D3  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_G)
#define D4  (SEG_B + SEG_C + SEG_F + SEG_G)
#define D5  (SEG_A + SEG_C + SEG_D + SEG_F + SEG_G)
#define D6  (SEG_A + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G)
#define D7  (SEG_A + SEG_B + SEG_C)
#define D8  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G)
#define D9  (SEG_A + SEG_B + SEG_C + SEG_D + SEG_F + SEG_G)


#define DMASK   ~(SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G)

const unsigned int digits[10] = {D0, D1, D2, D3, D4, D5, D6, D7, D8, D9};

int mpl=0;
unsigned int memory [1000] = {0};
unsigned int i;


unsigned int duty=0;
unsigned int frec=10000;

int mem;

unsigned int sin[64] = {140, 152, 165, 176, 188, 198, 208, 218, 226, 234, 240, 245, 250, 253, 254, 255, 254, 253, 250, 245, 240, 234, 226, 218, 208, 198, 188, 176, 165, 152, 140, 128, 115, 103, 90, 79, 67, 57, 47, 37, 29, 21, 15, 10, 5, 2, 1, 0, 1, 2, 5, 10, 15, 21, 29, 37, 47, 57, 67, 79, 90, 103, 115, 127, } ;

unsigned int tri[64] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0,  };

unsigned int tri2[15] = {1,2,3,4,5,7,8,9,10,11,12,13,14,15};


unsigned int fonda = 1;
unsigned int range=1;

unsigned int indice=0;

unsigned int atual=0;

// Variaveis para debounce
int count=0,state=0;
int count2=0,state2=0;


// Range de Frequencia


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Saida 7seg

    P3DIR = 0xFF;
    P3OUT = 0x00;


    P2OUT = 0x00;
    P2DIR |= BIT5 + BIT4;

    P8DIR |= BIT0 + BIT1;

    // saida dac

    P1DIR = BIT5 | BIT4 | BIT3 | BIT2;

    P4DIR = 0x0F;

    // Configuracao do botao P2.1

    P2REN|=BIT1; //Ativa o resistor de pull/up
    P2OUT|=BIT1; //Seta como resistor de pull/up

    P2IE|=BIT1; //Ativa a interrupção para o botao
    P2IES|=BIT1;



    // Saida DAC

    P1DIR |= BIT2+BIT3+BIT4+BIT5;
    P1OUT &= ~(BIT2+BIT3+BIT4+BIT5);

    P2DIR |= BIT5+BIT6;
    P2OUT &= ~(BIT5+BIT6);

    P2DIR |= pwm_out;           // Atribui a direcao de saida do pwm do pino 2.0
    P2SEL |= pwm_out;           // Seleciona a funcao especifica do pino

    // Configuracao Timer A1
/*
    TA1CCTL1 = OUTMOD_7;        // Modo de Reset/Set do timer
    TA1CCR0 = 10000;
    // Valor inicial do Timer para divisao do SMCLK (Modifica a frequencia)
    TA1CCR1 = 0;                // Valor inicial para o Duty (Modifica o Duty Cyclo)

    TA1CTL = MC_1 | TASSEL_2;   // Conta no modo UP e seleciona o SMCLK*/


    TA0CCR0 = 10000;
    TA0CCTL0 = CCIE;// Valor inicial do Timer para divisao do SMCLK (Modifica a frequencia)
    TA0CTL = MC_1 | TASSEL_2;   // Conta no modo UP e seleciona o SMCLK


    volatile unsigned int i;

    //Configuracao ADC12

    P6SEL |= BIT0 | BIT1;       // Habilita os canais de entrada analogica A0
    REFCTL0 &= ~REFMSTR;
    // Reseta o REFMSTR. Valor de referencia dos potenciometros

    ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT02+ADC12REFON+ADC12REF2_5V;
    // Ativacao do conversor analogico-digital.

    ADC12IE =  ADC12IE0 | ADC12IE1;      // Habilita as interrupcoes dos ADC's
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_3 ;// Multiplos canais e habilita a amostragem
    ADC12CTL2 = ADC12RES_0;              // Resolucao de 8 Bits

    ADC12MCTL0 = ADC12SREF_1;
    // Utiliza a referencia de 2,5v de entrada para o primeiro potenciometro
    ADC12MCTL1 = ADC12SREF_1 | ADC12INCH_1;
    // Referencia de 2,5v para o segundo, seleciona o canal 6.1 de entrada.

    for ( i=0; i<0x30; i++);             // Delay para inicializacao da referencia


    ADC12CTL0 |= ADC12ENC | ADC12SC;     // Habilita as conversoes ADC.

    // Configuracao do botao P1.1

    P1DIR&=~BIT1; //Coloca P1.1 como saida
    P1REN|=BIT1; //Ativa o resistor de pull/up
    P1OUT|=BIT1; //Seta como resistor de pull/up

    P1IE|=BIT1; //Ativa a interrupção para o botao
    P1IES|=BIT1; //Coloca como borda de descida do botao

    // Configuracao Timer A0


    TA0CCTL1 |= CCIE;
    TA0CCTL2 |= CCIE;
    TA0CCTL3 |= CCIE;
    TA0CCTL4 |= CCIE;

    TA0CCR1=1000;  //Contador de 0 a 1000  ~ 0.001 segundos
    TA0CCR2=1000;
    TA0CCR3=500;
    TA0CCR4=5000;



    __enable_interrupt();                // Habilita as interrupcoes

    while(1){
        while (!(ADC12IFG & BIT0));
        LPM0;                            //Low Power Mode 0
}

}
#pragma vector=ADC12_VECTOR
    __interrupt void OndaQuadrada (void){
        switch (__even_in_range(ADC12IV,8)) // Caracteriza prioridades de interrupcoes.
        {
            case  0: break;                           // Vector  0:  No interrupt
            case  2: break;                           // Vector  2:  ADC overflow
            case  4: break;                           // Vector  4:  ADC timing overflow
            case  6:

                mem = ADC12MEM0;
                /*
               // Atribui a variacao do potenciometro para a variavel Duty
               TA1CCR1 = duty;              // Atribui o valor de duty para o Timer 1
               ADC12CTL0 |= ADC12SC;        // Habiita novamente as conversoes*/
            break;

            case  8:

               frec = 1000 - (1000/250)*ADC12MEM1;
               // Atribui a variacao do potenciometro para a variavel frec
               TA0CCR0 = frec;              // Atribui o valor de frec para o Timer 2
               ADC12CTL0 |= ADC12SC;        // Habiita novamente as conversoes

            break;
            case 10: break;                           // Vector 10:  ADC12IFG2
            case 12: break;                           // Vector 12:  ADC12IFG3
            case 14: break;                           // Vector 14:  ADC12IFG4
            case 16: break;                           // Vector 16:  ADC12IFG5
            case 18: break;                           // Vector 18:  ADC12IFG6
            case 20: break;                           // Vector 20:  ADC12IFG7
            case 22: break;                           // Vector 22:  ADC12IFG8
            case 24: break;                           // Vector 24:  ADC12IFG9
            case 26: break;                           // Vector 26:  ADC12IFG10
            case 28: break;                           // Vector 28:  ADC12IFG11
            case 30: break;                           // Vector 30:  ADC12IFG12
            case 32: break;                           // Vector 32:  ADC12IFG13
            case 34: break;                           // Vector 34:  ADC12IFG14
            default: break;
        }


}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TMR0()
{

    switch( TA0IV ){

  
    case 2: //debounce B1

        break;


            __delay_cycles(1000);
            if(state==(P1IN&BIT1)) {
                count++; //incrementa se permanecer a mesma entrada
            } else
            {
                count=0; //se nao reseta o contador
                state=((P1IN&BIT1));
            }

        if(count==5){ //Conta x10 o valor atribuido ao timer
            if(state==0){//Botao precionado
                if(range==5){
                    range=1;
                } else{
                    range++;
                }
            }
            P1IE|=BIT1; //Ativa novamente a interrupcao

            }

        TA0CTL&=~(TAIFG); //Reseta a interrupcao


    case 4: i++ ;

            if (i==1000) i = 0;

            memory[i] = atual;

            
    case 6: //debounce B2
        break;

        __delay_cycles(1000);
        if(state2==(P2IN&BIT1)) {
                        count2++; //incrementa se permanecer a mesma entrada
                    } else
                    {
                        count2=0; //se nao reseta o contador
                        state=((P2IN&BIT1));
                    }

                if(count2==10){ //Conta x10 o valor atribuido ao timer
                    if(state2==0){//Botao precionado

                        if(fonda==2){
                            fonda=1;
                        } else{
                            fonda++;
                        }
                    }
                    P2IE|=BIT1; //Ativa novamente a interrupcao

                    }

                    TA0CTL&=~(TAIFG); //Reseta a interrupcao


    case 8: //multiplexacao Displays 7seg

        if (mpl == 1){
            mpl = 0;
            P8OUT = 1;
            P3OUT = digits[range];

        }

        else {
            mpl=1;
            P8OUT = 2;
            P3OUT = digits[fonda];
        }

    default: break;
}}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{

    /*if (fonda == 1) {atual = sin[indice];}
    else {atual = tri[indice];}
     */

    if (indice == 14) {indice = 0;}
    else {indice++;}

    atual = tri2[indice]*2;

    P1OUT = (atual&0xF0)*4;
    P4OUT = (atual&0x0F);


}


#pragma vector=PORT1_VECTOR
__interrupt void P1_Function()
{
    count=0; //Reset count
    state=(P1IN&BIT1); //Armazena o valor do Botao
    //while(!(P1IN & BIT0));
    P1IE&=~BIT1; //Desabilita a interrupcao
    P1IFG&=~BIT1; // Reset Port1 interrupt flag
}

#pragma vector=PORT2_VECTOR
__interrupt void P2_Function()
{
    
    count2=0; //Reset count
    state2=(P2IN&BIT1); //Armazena o valor do Botao
    fonda++;

    P2IE&=~BIT1; //Desabilita a interrupcao*/
    P2IFG&=~BIT1; // Reset Port1 interrupt flag
}
