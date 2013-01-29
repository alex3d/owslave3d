#include "config.h"

#ifndef NO_INTERRUPTS

#ifdef EXCLUSIVE_TRANSCEIVE
#include "owslave_fastest.h"
#else
#include "owslave.h"
#endif

void delay(int ms){
    do{
    _delay_ms(10);
    ms-=10;
    }while(ms>0);
}
#define _delay_ms delay

void checkReset(){


    uint8_t reset = MCUSR;
    MCUSR = 0;
    if(reset==_BV(PORF)){
        PORTD |= (1<<5);
        _delay_ms(300);
        PORTD &= ~(1<<5);
        _delay_ms(300);

        PORTD |= (1<<5);
        _delay_ms(300);
        PORTD &= ~(1<<5);
        _delay_ms(300);

        PORTD |= (1<<5);
        _delay_ms(1000);
        PORTD &= ~(1<<5);
        return;
    }
    if(reset==_BV(BORF)){
        while(1){
            PORTD |= (1<<5); //1
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);

            _delay_ms(2000);
        }
    }
    if(reset==_BV(EXTRF)){
        while(1){
            PORTD |= (1<<5); //2
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);
            PORTD |= (1<<5);
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);

            _delay_ms(2000);
            return;
        }
    }
    if(reset==_BV(WDRF)){
        while(1){
            PORTD |= (1<<5); //3
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);
            PORTD |= (1<<5);
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);
            PORTD |= (1<<5);
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);

            _delay_ms(2000);
            return;
        }
    }
    if(reset==0){
        while(1){
            PORTD |= (1<<5); // 4
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);
            PORTD |= (1<<5);
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);
            PORTD |= (1<<5);
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);
            PORTD |= (1<<5);
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);

            _delay_ms(2000);
            return;
        }
    }
    while(1){
            PORTD |= (1<<5);
            _delay_ms(300);
            PORTD &= ~(1<<5);
            _delay_ms(300);
    }
}
/*
ISR(BADISR_vect){
    while(1){
        PORTD |= (1<<5);
        _delay_ms(300);
        PORTD &= ~(1<<5);
        _delay_ms(300);
        PORTD |= (1<<5);
        _delay_ms(1000);
        PORTD &= ~(1<<5);
        _delay_ms(1000);
    }
}
*/
int main(void) {
    states_init();
    owslave_init();
    idle_state=0;
    owslave_value=0;
    owslave_bitmask=0;
    // debug pin
    DDRD |= (1<<5);
    PORTD &= ~(1<<5);

    checkReset();
    sei();
    while(1){
        /*
        _delay_ms(1000);
        PORTD |= (1<<5);
        _delay_ms(1000);
        PORTD &= ~(1<<5);
        */

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

