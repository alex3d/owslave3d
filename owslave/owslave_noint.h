enum State{
    IDLE,
    RESET_WAIT,
    PRESENCE_RAISE_WAIT, // ждём presence raise 
    PRESENCE_WAIT,       // ждём presence
    PRESENCE,            // презентуем
    STATE_FIRST,
};

#define OWSLAVE_STATES_GLOBAL
#include <owslave_states.h>

//void onewire_noint_main(void){
int main(void) __attribute__((noreturn)) __attribute__ ((OS_main));
//int main(void) __attribute__((naked));
int main(void){
//register uint16_t addr asm("r10");
//register union{uint16_t i16; struct{uint8_t l, h;}; } addru asm("r10");
union{uint16_t i16; struct{uint8_t l, h;}; } addru, owslave_state_temp_word;
#define owslave_state_temp owslave_state_temp_word.l
#define owslave_state_temp1 owslave_state_temp_word.h
//union{uint16_t owslave_state_temp_word; struct{uint8_t owslave_state_temp, owslave_state_temp1;}; };


    uint8_t owslave_state;
    void* owslave_state_label;
    uint8_t owslave_bitmask;
    uint8_t owslave_value;
    //register void* state_label asm("r30");
//    uint8_t owslave_state_temp;
    uint8_t* owslave_state_temp_ptr;

    asm("eor r1,r1 ");

    states_init();
    owslave_init();
/*
    PORTC |= (1<<5);
    _delay_ms(1000);
    PORTC &= ~(1<<5);
*/
IDLE:
    owslave_pin_falling();
    while(!owslave_pin_int());
    owslave_pin_clear_int();
    owslave_set_timeout(T_RESET);
    owslave_timer_clear_int();
    owslave_idle();
    do{
        if(owslave_pin_int()) goto IDLE;
    }while(!owslave_timer_int());
    if(owslave_pin_value()) goto IDLE;
PRESENCE_RAISE_WAIT:
    owslave_pin_rising();
    owslave_set_timeout(T_RESET);
    owslave_pin_clear_int();
    owslave_timer_clear_int();
    do{
    //while(!owslave_pin_int());
        if(owslave_timer_int())
            goto IDLE;
    }while(!owslave_pin_int());
    owslave_pin_falling();
//PRESENCE_WAIT:
    owslave_set_timeout(T_PRESENCEWAIT);
    owslave_timer_clear_int();
    while(!owslave_timer_int());
//PRESENCE:
    owslave_pin_low();
    owslave_set_timeout(T_PRESENCE);
    owslave_timer_clear_int();
    while(!owslave_timer_int());
    owslave_pin_hiz();
//ROM_COMMAND:
    owslave_pin_clear_int();
    DO_RECEIVE_TO_STATE(ROM_COMMAND);
TRANSCEIVE:
        owslave_set_timeout(T_RESET);
        owslave_timer_clear_int();
        while(!owslave_pin_int())
            if(owslave_timer_int()){
                if(!owslave_pin_value()) goto PRESENCE_RAISE_WAIT;
                owslave_set_timeout(T_RESET);
                owslave_timer_clear_int();
            }
        if(!(owslave_value&owslave_bitmask))
            owslave_pin_low();

    __builtin_avr_delay_cycles(TICKS_IN_SAMPLE-12);
    
    owslave_value |= owslave_bitmask;
    if(!owslave_pin_value())
        owslave_value ^= owslave_bitmask;
        
    owslave_pin_clear_int();
    owslave_pin_hiz();
    owslave_bitmask <<= 1;
    if(owslave_bitmask) goto TRANSCEIVE;

    BEGIN_STATES
    #define OWSLAVE_STATES_STATES
    #include <owslave_states.h>
    END_STATES
    goto TRANSCEIVE;
}

