#include "config.h"

#ifndef NO_INTERRUPTS

#ifdef EXCLUSIVE_TRANSCEIVE
#include "owslave_fastest.h"
#else
#include "owslave.h"
#endif

//EMPTY_INTERRUPT(INT1_vect);
//ISR_ALIAS(INT1_vect, INT0_vect);
ISR(INT1_vect, ISR_ALIASOF(INT0_vect));

void delay(int ms){
    do{
    _delay_ms(10);
    ms-=10;
    }while(ms>0);
}
//#define _delay_ms delay

int main(void) {

    states_init();
    owslave_init();
    idle_state=0;
    owslave_value=0;
    owslave_bitmask=0;
    // debug pin
    DDRD |= (1<<5);
    PORTD &= ~(1<<5);

        _delay_ms(100);
        PORTD |= (1<<5);
        _delay_ms(100);
        PORTD &= ~(1<<5);

        _delay_ms(100);
        PORTD |= (1<<5);
        _delay_ms(100);
        PORTD &= ~(1<<5);

        _delay_ms(1000);
        PORTD |= (1<<5);
        _delay_ms(1000);
        PORTD &= ~(1<<5);

    sei();
    while(1){
        _delay_ms(1000);
        PORTD |= (1<<5);
        _delay_ms(1000);
        PORTD &= ~(1<<5);

        owslave_idle();
    }
}
#else

#include "owslave_noint.h"
/*
int main(void){
    onewire_noint_main();
}
*/
#endif

