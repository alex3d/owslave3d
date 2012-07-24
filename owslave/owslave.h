enum IdleState{
    IDLE=0,
    RESET_WAIT,
    PRESENCE_RAISE_WAIT,
    PRESENCE_WAIT,
    PRESENCE,
    STATE_FIRST,
};

#define OWSLAVE_STATES_GLOBAL
#include <owslave_states.h>

#define idle_state owslave_state

owslave_timer_interrupt{
    if(!(idle_state&0x80))
        goto CHECK_IDLE_STATE;
    // sample next bit
    idle_state &= 0x7F;
    owslave_pin_hiz();
    owslave_unmask_pin();
    owslave_value |= owslave_bitmask;
    if(!owslave_pin_value())
        owslave_value ^= owslave_bitmask;
       
    if(!(owslave_bitmask&0x80))
        goto TRANSCEIVE_NEXT;

    BEGIN_STATES
    #define OWSLAVE_STATES_STATES
    #include <owslave_states.h>
    END_STATES

    IDLE:
    owslave_bitmask = 0;
    idle_state = IDLE;
    //idle_state = RESET_WAIT;
    return;



    CHECK_IDLE_STATE:
    if(owslave_bitmask){
        if(!owslave_pin_value())
            goto timeout;
        return;
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
        idle_state=RESET_WAIT;
        DO_RECEIVE_TO_STATE(ROM_COMMAND);
        break;
    case IDLE:
        break;
    case RESET_WAIT:
    //default: //case RESET_WAIT:
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
    return;



    TRANSCEIVE_NEXT:
    owslave_bitmask <<= 1;

    TRANSCEIVE:
    owslave_set_timeout(T_RESET);
    //idle_state&=0x7F;
    return;

}

owslave_pin_interrupt{
    if(owslave_bitmask){
        if(!(owslave_value & owslave_bitmask)) owslave_pin_low();
        owslave_set_timeout(T_SAMPLE);
        owslave_timer_clear_int(); // clear interrupt if it is already waiting TODO: really need this?
        owslave_mask_pin();
        idle_state |= 0x80;
        return;
    }
    switch(idle_state){
    default: // IDLE, RESET_WAIT
        owslave_set_timeout(T_RESET);
        owslave_timer_clear_int(); // clear interrupt if it is already waiting TODO: really need this?
        idle_state = RESET_WAIT;
        break;
    case PRESENCE_RAISE_WAIT:
        owslave_set_timeout(T_PRESENCEWAIT);
        owslave_timer_clear_int(); // clear interrupt if it is already waiting TODO: really need this?
        owslave_mask_pin();
        owslave_pin_falling();
        idle_state = PRESENCE_WAIT;
        break;
    }
}

