enum IdleState{
    IDLE=0,
    PRESENCE_RAISE_WAIT=1,
    PRESENCE_WAIT,
    PRESENCE,
};

#define OWSLAVE_STATES_GLOBAL
#include <owslave_states.h>

#define idle_state owslave_bitmask

owslave_timer_interrupt{
   if(owslave_state_label==0){
		if(!(idle_state&0x2)){
            if(owslave_pin_value()) return;
			owslave_pin_rising();
			idle_state = PRESENCE_RAISE_WAIT;
		}else{
			if(!(idle_state&1)){ // PRECENCE_WAIT
				owslave_set_timeout(T_PRESENCE);
	            owslave_pin_low();
	            idle_state = PRESENCE;
	        }else{ // PRESENCE
				owslave_pin_falling();
	            owslave_pin_hiz();
	            owslave_unmask_pin();
	            DO_RECEIVE_TO_STATE(ROM_COMMAND);
	        }
        }
    }else{
		if(owslave_pin_unmasked()){
			if(owslave_pin_value())
				return;
	        IDLE:
	        owslave_state_label=0;
	        idle_state = IDLE;
	        return;
		}
        // sample next bit
        owslave_pin_hiz();
		owslave_unmask_pin();
        owslave_value |= owslave_bitmask;
        if(!owslave_pin_value())
            owslave_value ^= owslave_bitmask;
        
		owslave_bitmask <<= 1;		
        if(owslave_bitmask) goto TRANSCEIVE;
		
		BEGIN_STATES
		#define OWSLAVE_STATES_STATES
		#include <owslave_states.h>
		END_STATES
		
		RECEIVE:
		owslave_value=0xFF;
		TRANSCEIVE_BYTE:
		owslave_bitmask=1;
        TRANSCEIVE:
        owslave_set_timeout(T_RESET);
    }
}

owslave_pin_interrupt{
    if(owslave_state_label!=0){
        if(!(owslave_value & owslave_bitmask)) owslave_pin_low();
        owslave_mask_pin();
        owslave_set_timeout(T_SAMPLE);
    }else if(idle_state&1){
        idle_state = PRESENCE_WAIT;
		owslave_mask_pin();
        owslave_set_timeout(T_PRESENCEWAIT);
    }else{
		owslave_set_timeout(T_RESET);
    }
}

