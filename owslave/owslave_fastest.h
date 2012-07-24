enum State{
    IDLE,
    RESET_WAIT,
    PRESENCE_RAISE_WAIT, // ждём presence raise 
    PRESENCE_WAIT,       // ждём presence
    PRESENCE,            // презентуем
    STATE_FIRST,
};

#define OWSLAVE_STATES_GLOBAL
#include "owslave_states.h"

#define idle_state owslave_state

owslave_timer_interrupt{
    if(owslave_bitmask){
        if(!(idle_state&0x80)){
            if(!owslave_pin_value()){
                owslave_bitmask=0;
                goto timeout;
            }
            return;
        }
    }

    switch(idle_state){
    case PRESENCE_RAISE_WAIT: // this is TIMEOUT FIXME: really timeout?
        owslave_pin_falling();
        idle_state = IDLE;
        break;
    case PRESENCE_WAIT:
        owslave_set_timeout(T_PRESENCE);
        owslave_pin_low();
        idle_state = PRESENCE;
        break;
    case PRESENCE:
        owslave_pin_hiz();
        owslave_unmask_pin();
        owslave_bitmask=1;
        owslave_value=0xFF;
        owslave_set_timeout(T_RESET);
        break;
    case IDLE:
        break;
    case RESET_WAIT:
        if(owslave_pin_value()){
            idle_state = IDLE;
            break;
        }
    timeout:
        owslave_bitmask=0;
        owslave_set_timeout(T_RESET);
        owslave_pin_rising();
        owslave_pin_clear_int();
        idle_state = PRESENCE_RAISE_WAIT;
    break;
    }
}

owslave_pin_interrupt{
    if(owslave_bitmask){
        void* owslave_state_label = &&ROM_COMMAND;
        uint8_t local_bitmask=1;
        uint8_t owslave_value=0xFF;

        uint8_t owslave_state_temp;
        uint8_t* owslave_state_temp_ptr;

        #define search_byte search_value

        #define match_byte search_value
        #define read_scratchpad_byte search_value
        #define match_ptr search_ptr
        #define owslave_bitmask local_bitmask

        goto TRANSCEIVE_RECEIVE;

        TRANSCEIVE_NEXT: ;
        owslave_bitmask <<= 1;

        TRANSCEIVE:
        if(!(owslave_value&owslave_bitmask)){ // on 1MHz we don't have enougth time for this check between pin_int and pin_low
            owslave_set_timeout(T_RESET);
            owslave_timer_clear_int();
            while(!owslave_pin_int())
                if(owslave_timer_int()){
                    if(owslave_pin_int()) break;
                    owslave_set_timeout(T_RESET);
                    if(owslave_pin_int()) break;
                    owslave_timer_clear_int();
                    if(owslave_pin_int()) break;
                    if(!owslave_pin_value()) goto timeout;
                }
            owslave_pin_low();
        }else{
            owslave_set_timeout(T_RESET);
            owslave_timer_clear_int();
            while(!owslave_pin_int())
                if(owslave_timer_int()){
                    if(owslave_pin_int()) break;
                    owslave_set_timeout(T_RESET);
                    if(owslave_pin_int()) break;
                    owslave_timer_clear_int();
                    if(owslave_pin_int()) break;
                    if(!owslave_pin_value()) goto timeout;
                }
        }
        __builtin_avr_delay_cycles(TICKS_IN_SAMPLE-8);

        TRANSCEIVE_RECEIVE:
        owslave_value |= owslave_bitmask;
        if(!owslave_pin_value())
            owslave_value ^= owslave_bitmask;
        owslave_pin_clear_int();
        owslave_pin_hiz();
        if(!(owslave_bitmask&0x80)) goto TRANSCEIVE_NEXT;

        STATES:
        BEGIN_STATES
        #define OWSLAVE_STATES_STATES
        #include "owslave_states.h"
        END_STATES

        goto TRANSCEIVE;
        
        
        

    timeout: // TODO: check me
#undef owslave_bitmask
        owslave_bitmask=0;
        owslave_set_timeout(T_RESET);
        owslave_pin_rising();
        owslave_pin_clear_int();
        idle_state = PRESENCE_RAISE_WAIT;
        return;

        IDLE:
        idle_state = IDLE;
        owslave_bitmask = 0;
        return;
    }else{
        switch(idle_state){
        default: // IDLE, RESET_WAIT
            owslave_set_timeout(T_RESET);
            idle_state = RESET_WAIT;
            break;
        case PRESENCE_RAISE_WAIT:
            owslave_set_timeout(T_PRESENCEWAIT);
            owslave_mask_pin();
            owslave_pin_falling();
            idle_state = PRESENCE_WAIT;
            break;
        }
    }
}
