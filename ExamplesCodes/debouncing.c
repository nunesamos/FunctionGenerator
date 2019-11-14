
#include <msp430f5529.h>

int count=0,state;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    P1DIR&=~BIT1; //Coloca P1.1 como saida
    P1REN|=BIT1; //Ativa o resistor de pull/up
    P1OUT|=BIT1; //Seta como resistor de pull/up

    P1IE|=BIT1; //Ativa a interrupção para o botao
    P1IES|=BIT1; //Coloca como borda de descida do botao

    TA0CCR0=1000; //Contador de 0 a 1000  ~ 0.001 segundos
    __enable_interrupt(); //Habilita a interrupção

    P1DIR|=BIT0; //P1.0 como saida
    P1OUT&=~BIT0; //Inicializa com o Led1 desligado

    P4DIR|=BIT7; //P4.7 como saida

   while(1)
   {
       P4OUT^=BIT7; //Toggle P4.7
       __delay_cycles(200000); //Delay

   }

}

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
            if(state==0) //Botao precionado
                P1OUT^=BIT0; //Toggle the LED
            P1IE|=BIT1; //Ativa novamente a interrupcao
            TA0CTL=0; //para o timer
            TA0CTL|=TACLR; //Clear the Timer counter
            }

            TA0CTL&=~(TAIFG); //Reseta a interrupcao
}
