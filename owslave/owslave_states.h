#ifdef OWSLAVE_STATES_GLOBAL
enum{
    OW_SEARCH_ROM=0xF0,
    OW_READ_ROM=0x33,
    OW_MATCH_ROM=0x55,
    OW_SKIP_ROM=0xCC,
    OW_ALARM_SEARCH=0xEC,
};


#if USE_COMPUTED_GOTO
#define BEGIN_STATES do{ goto *owslave_state_label;
#define END_STATES }while(0);
#define STATE(s) s: ;
#define STATE_END break;
#define DO_RECEIVE_TO_STATE(st) do{ owslave_state_label=&&st; owslave_value=0xFF; owslave_bitmask=1; goto TRANSCEIVE; }while(0)
#define DO_RECEIVE_TO_SELF(st) do{ owslave_value=0xFF; owslave_bitmask=1; goto TRANSCEIVE; }while(0)
#define DO_TRANSCEIVE_TO_SELF(st, val, bm) do{ owslave_value=val; owslave_bitmask=bm; goto TRANSCEIVE; }while(0)
#define DO_TRANSCEIVE_TO_STATE(st, val, bm) do{ owslave_state_label=&&st; owslave_value=val; owslave_bitmask=bm; goto TRANSCEIVE; }while(0)
#define SET_STATE(st) owslave_state_label=&&st
#define SET_IDLE goto IDLE
#else
enum{
    STATE_DUMMY=STATE_FIRST,
    ROM_COMMAND,
    MATCH_ROM,
    SEARCH,
    STATE_FIRST_CUSTOM,
};
#define BEGIN_STATES switch(owslave_state){
#define END_STATES END_STATE: ;}
#define STATE(s) case s: s: ;
#define STATE_END goto END_STATE;
#define DO_RECEIVE_TO_STATE(st) do{ owslave_state=st; owslave_value=0xFF; owslave_bitmask=1; goto TRANSCEIVE;}while(0)
#define DO_RECEIVE_TO_SELF(st) do{ owslave_value=0xFF; owslave_bitmask=1; goto TRANSCEIVE;}while(0)
#define DO_TRANSCEIVE_TO_SELF(st, val, bc) do{ owslave_value=val; owslave_bitmask=bc; goto TRANSCEIVE;}while(0)
#define SET_IDLE goto IDLE
#define SET_STATE(st) owslave_state=st
#endif

#include <owslave_custom_states.h>
#undef OWSLAVE_STATES_GLOBAL

#endif

#ifdef OWSLAVE_STATES_STATES
STATE(ROM_COMMAND)
    switch(owslave_value){
    case OW_SEARCH_ROM:
        goto SEARCH_INIT;
    case OW_MATCH_ROM:
        goto MATCH_ROM_INIT;
    default:
        SET_IDLE;
    }
STATE_END

STATE(MATCH_ROM)
#ifdef USE_PTR
    if(owslave_value!=*(owslave_state_temp_ptr))
        SET_IDLE;
    if((++owslave_state_temp_ptr)>=UID+8)
        DO_RECEIVE_TO_STATE(DEVICE_COMMAND);
    else
        DO_RECEIVE_TO_SELF(MATCH_ROM);
#else
    if(owslave_value!=UID[owslave_state_temp])
        SET_IDLE;
    if((++owslave_state_temp)&8)
        DO_RECEIVE_TO_STATE(DEVICE_COMMAND);
    else
        DO_RECEIVE_TO_SELF(MATCH_ROM);
#endif

MATCH_ROM_INIT:
#ifdef USE_PTR
    owslave_state_temp_ptr = UID;
#else
    owslave_state_temp = 0;
#endif
    DO_RECEIVE_TO_STATE(MATCH_ROM);
STATE_END

STATE(SEARCH)
    #define search_bit owslave_state_temp
#if defined(EXCLUSIVE_TRANSCEIVE) || defined(NO_INTERRUPTS) // temp is local
    #define search_bit_local owslave_state_temp
    #define search_byte owslave_value
#else
    uint8_t search_bit_local;
    uint8_t search_byte;
#endif
    //#define value search_byte
    if( ((owslave_value>>4)^owslave_value)&(1<<3) ) // >>4 faster than <<3 on AVR
         SET_IDLE;
#ifndef USE_PTR
    search_byte= owslave_value&0x7;
#endif
    search_bit_local=search_bit;
    search_bit_local <<= 1;
    if(!search_bit_local){
#ifdef USE_PTR
        if((++owslave_state_temp_ptr)>=(UID+8))
            SET_IDLE;
#else
        if((++search_byte)&8)
            SET_IDLE;
#endif
    SEARCH_1:
        search_bit_local=1;
    }
    search_bit = search_bit_local;
    
#ifdef USE_PTR
    if((*owslave_state_temp_ptr)&search_bit_local)
        DO_TRANSCEIVE_TO_SELF(SEARCH, 0xB8, 0x20);
    else
        DO_TRANSCEIVE_TO_SELF(SEARCH, 0xC0, 0x20);
#else
    if((UID[search_byte])&search_bit_local)
       search_byte |= 0xB8;
    else
       search_byte |= 0xC0;
    DO_TRANSCEIVE_TO_SELF(SEARCH, search_byte, 0x20);
#endif
SEARCH_INIT: ;
#ifdef USE_PTR
    owslave_state_temp_ptr = UID;
#else
    search_byte = 0;
#endif
    SET_STATE(SEARCH);
    goto SEARCH_1;

#undef search_byte 
STATE_END

#include <owslave_custom_states.h>

#undef OWSLAVE_STATES_STATES
#endif
