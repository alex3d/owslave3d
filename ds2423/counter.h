#include <avr/io.h>

uint32_t uptime=0;

uint32_t counter=0;
uint32_t counterFast=0;

void counter_init(){
    //TCCR1B = 2; // prescaler 8
    TCCR1B = 5; // prescaler 1024

    MCUCR |= (2 << ISC10);  // Interrupt on falling int1
    DDRD &= ~(1<<3); // int1 input
    PORTD |= (1<<3); // int1 pullup
    GIMSK |= 1<<INT1;
    TIMSK |= 1<<TOIE1;
}

uint32_t counter_get(uint8_t index){
//    counter++;
    return counter;
}

ISR(INT1_vect){
    if(TIMSK&(1<<OCIE1A)){
        counterFast++;
        return;
    }
    counter++;
    OCR1A = TCNT1 + 10000; // 10ms
    TIFR = 1<<OCF1A;
    TIMSK |= 1<<OCIE1A;
    PORTD |= (1<<5);
}

ISR(TIMER1_COMPA_vect){
    TIMSK &= ~(1<<OCIE1A);
    PORTD &= ~(1<<5);
}

ISR(TIMER1_OVF_vect){
    //uptime++;
    //counter++;
}
