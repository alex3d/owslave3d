#ifdef OWSLAVE_STATES_GLOBAL
enum{
    OW_SEARCH_ROM=0xF0,
    OW_READ_ROM=0x33,
    OW_MATCH_ROM=0x55,
    OW_SKIP_ROM=0xCC,
    OW_ALARM_SEARCH=0xEC,
};

#define BEGIN_STATES do{ goto *owslave_state_label;
#define END_STATES }while(0);
#define STATE(s) s: ;
#define STATE_END break;
#define DO_RECEIVE_TO_STATE(st) do{ owslave_state_label=&&st; owslave_value=0xFF; owslave_bitmask=1; goto TRANSCEIVE; }while(0)
#define DO_RECEIVE_TO_SELF(st) do{ goto RECEIVE; }while(0)
#define DO_TRANSCEIVE_TO_SELF(st, val, bm) do{ owslave_value=val; owslave_bitmask=bm; goto TRANSCEIVE; }while(0)
#define DO_TRANSCEIVE_TO_STATE(st, val, bm) do{ owslave_state_label=&&st; owslave_value=val; owslave_bitmask=bm; goto TRANSCEIVE; }while(0)

#define SEND(val) { __label__ xxx; DO_TRANSCEIVE_TO_STATE(xxx, val, 1); xxx: ; }
#define SENDCRC(val) { __label__ xxx; DO_TRANSCEIVE_TO_STATE1(xxx, val, 1, crc16()); xxx: ; }
#define RECEIVE() SEND(0xFF)


#define SET_STATE(st) owslave_state_label=&&st
#define SET_IDLE goto IDLE

#include <owslave_custom_states.h>
#undef OWSLAVE_STATES_GLOBAL

#endif

#ifdef OWSLAVE_STATES_STATES
STATE(ROM_COMMAND)
    switch(owslave_value){
	default:
        SET_IDLE;
    case OW_SEARCH_ROM:
        goto SEARCH_INIT;
    case OW_MATCH_ROM:
		for(owslave_state_temp=0; !(owslave_state_temp&8); owslave_state_temp++){
			RECEIVE();
	        if(owslave_uid_byte(owslave_state_temp)!=owslave_value) SET_IDLE;
		}
		DO_RECEIVE_TO_STATE(DEVICE_COMMAND);
    }
STATE_END

STATE(SEARCH)
    #define search_bit owslave_state_temp
#if defined(EXCLUSIVE_TRANSCEIVE) || defined(NO_INTERRUPTS) // temp is local
    #define search_bit_local owslave_state_temp
    #define search_byte owslave_value
#else
//#define search_bit_local owslave_state_temp
    uint8_t search_bit_local;
	//#define search_byte owslave_value
    uint8_t search_byte;
#endif
    //#define value search_byte
    if( ((owslave_value>>4)^owslave_value)&(1<<3) ) // >>4 faster than <<3 on AVR
         SET_IDLE;
    search_byte= owslave_value&0x7;
    search_bit_local=search_bit;
    search_bit_local <<= 1;
    if(!search_bit_local){
        if((++search_byte)&8)
            SET_IDLE;
    SEARCH_1:
        search_bit_local=1;
    }
    search_bit = search_bit_local;
    
    if(owslave_uid_byte(search_byte)&search_bit_local)
       search_byte |= 0xB8;
    else
       search_byte |= 0xC0;
    DO_TRANSCEIVE_TO_SELF(SEARCH, search_byte, 0x20);
	
SEARCH_INIT: ;
    search_byte = 0;
    SET_STATE(SEARCH);
    goto SEARCH_1;

#undef search_byte 
STATE_END

#include <owslave_custom_states.h>

#undef OWSLAVE_STATES_STATES
#endif
