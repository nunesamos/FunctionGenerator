#include <msp430f5529.h>

#define pwm_out BIT0

// Variaveis para PWM
unsigned int duty=0;
unsigned int frec=10000;

// Variaveis para debounce
int count=0,state;


// Range de Frequencia
int range=1;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

// -------------------------------- P W M ------------------------------------------

        P2DIR |= pwm_out;           // Atribui a direcao de saida do pwm do pino 2.0
        P2SEL |= pwm_out;           // Seleciona a funcao especifica do pino

        TA1CCTL1 = OUTMOD_7;        // Modo de Reset/Set do timer
        // Valor inicial do Timer para divisao do SMCLK (Modifica a frequencia)
        TA1CCR0 = 10000;
        TA1CCR1 = 0;                // Valor inicial para o Duty (Modifica o Duty Cyclo)

        TA1CTL = MC_1 | TASSEL_2;   // Conta no modo UP e seleciona o SMCLK


        volatile unsigned int i;

        P6SEL |= BIT0 | BIT1;       // Habilita os canais de entrada analogica A0
        // Reseta o REFMSTR. Valor de referencia dos potenciometros
        REFCTL0 &= ~REFMSTR;

        // Ativacao do conversor analogico-digital.
        ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT02+ADC12REFON+ADC12REF2_5V;


        ADC12IE =  ADC12IE0 | ADC12IE1;      // Habilita as interrupcoes dos ADC's
        ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;// Multiplos canais e habilita a amostragem
        ADC12CTL2 = ADC12RES_0;              // Resolucao de 8 Bits

        // Utiliza a referencia de 2,5v de entrada para o primeiro potenciometro
        ADC12MCTL0 = ADC12SREF_1;
        // Referencia de 2,5v para o segundo, seleciona o canal 6.1 de entrada.
        ADC12MCTL1 = ADC12SREF_1 | ADC12INCH_1;


        for ( i=0; i<0x30; i++);             // Delay para inicializacao da referencia


        ADC12CTL0 |= ADC12ENC | ADC12SC;     // Habilita as conversoes ADC.


// ------------------------------- D E B O U N C E -----------------------------------

        P1DIR&=~BIT1; //Coloca P1.1 como saida
        P1REN|=BIT1; //Ativa o resistor de pull/up
        P1OUT|=BIT1; //Seta como resistor de pull/up

        P1IE|=BIT1; //Ativa a interrupção para o botao
        P1IES|=BIT1; //Coloca como borda de descida do botao

        TA0CCR0=1000; //Contador de 0 a 1000  ~ 0.001 segundos


        __enable_interrupt(); //Habilita a interrupção

//      P1DIR|=BIT0; //P1.0 como saida
//      P1OUT&=~BIT0; //Inicializa com o Led1 desligado

//      P4DIR|=BIT7; //P4.7 como saida

// --------------------------------- Main Loop ---------------------------------------

    while(1){
        LPM1;                            //Low Power Mode 1
//       P4OUT^=BIT7; //Toggle P4.7
//       __delay_cycles(200000); //Delay
   }

}
//------------------------- Pragma Interrupt para o Debounce -----------------------------

#pragma vector=PORT1_VECTOR
__interrupt void P1_Function()
{
    count=0; //Reset count
    TA0CTL|=TASSEL_2+MC_1+TAIE; //Timer0 recebe ACLK
    state=(P1IN&BIT1); //Armazena o valor do Botao
    P1IE&=~BIT1; //Desabilita a interrupcao
    P1IFG&=~BIT1; // Reset Port1 interrupt flag
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TMR0()
{

            if(state==(P1IN&BIT1)) {
                count++; //incrementa se permanecer a mesma entrada
            } else
            {
                count=0; //se nao reseta o contador
                state=((P1IN&BIT1));
            }

        if(count==10){ //Conta x10 o valor atribuido ao timer
            if(state==0){//Botao precionado
//              P1OUT^=BIT0; //Toggle the LED
                if(range==5){
                    range=1;
                } else{
                    range++;
                }
            }
            P1IE|=BIT1; //Ativa novamente a interrupcao
            TA0CTL=0; //para o timer
            TA0CTL|=TACLR; //Clear the Timer counter
            }

            TA0CTL&=~(TAIFG); //Reseta a interrupcao
}


//------------------------- Pragma Interrupt para o PWM -----------------------------

#pragma vector=ADC12_VECTOR
    __interrupt void OndaQuadrada (void){
        switch (__even_in_range(ADC12IV,8)) // Caracteriza prioridades de interrupcoes.
        {
           case 0x06:
               // Atribui a variacao do potenciometro para a variavel Duty
               duty = (frec/250)*ADC12MEM0;
               TA1CCR1 = duty;              // Atribui o valor de duty para o Timer 1
               ADC12CTL0 |= ADC12SC;        // Habiita novamente as conversoes
//               ADC12IFG &=~(ADC12IFG0);           // Reseta a flag de interrupcao
           break;

           case 0x08:
               // Atribui a variacao do potenciometro para a variavel frec
               frec = (10000 - (10000/250)*ADC12MEM1)/range;
               TA1CCR0 = frec;              // Atribui o valor de frec para o Timer 2
               ADC12CTL0 |= ADC12SC;        // Habiita novamente as conversoes
//               ADC12IFG &=~(ADC12IFG1);           // Reseta a flag de interrupcao
           break;

           default:
           break;
        }
        ADC12IFG&=~(ADC12IFG0);           // Reseta a flag de interrupcao
        ADC12IFG&=~(ADC12IFG1);           // Reseta a flag de interrupcao
    }

