#ifdef OWSLAVE_STATES_GLOBAL

#define DO_TRANSCEIVE_TO_SELF1(st, val, bm, c) do{ owslave_value=val; owslave_bitmask=bm; c; goto TRANSCEIVE; }while(0)
#define DO_TRANSCEIVE_TO_STATE1(st, val, bm, c) do{ owslave_state_label=&&st; owslave_value=val; owslave_bitmask=bm; c; goto TRANSCEIVE; }while(0)

#define SEND(val) { __label__ xxx; DO_TRANSCEIVE_TO_STATE(xxx, val, 1); xxx: ; }
#define SENDCRC(val) { __label__ xxx; DO_TRANSCEIVE_TO_STATE1(xxx, val, 1, crc16()); xxx: ; }
#define RECEIVE() SEND(0xFF)

#include "avr/pgmspace.h"
#include "avr/boot.h"

enum{
    OW_READ_MEM_COUNTER = 0xA5,
};

inline void states_init(void){
    counter_init();
}

inline void owslave_idle(void){
}
uint16_t addr;
uint8_t addr8;
uint16_t crc;
uint32_t owslave_temp_counter=1000000;
uint8_t counter_idx;

void crc16(void) {
    union{uint16_t i16; struct{uint8_t l, h;}; } r, c;
    r.i16 = crc;
    c.l = owslave_value^r.l;
    r.i16 >>= 8;

    uint8_t cc=c.l;
    //cc ^= __builtin_avr_swap(cc);
    cc ^= cc>>4;
    cc ^= cc>>2;
    cc ^= cc>>1;
    if (cc&1){
        //r ^= 0xC001;
        r.l = r.l^1 ;
        r.h = 0xC0;
    }
    //r ^= (c <<= 6);
    //r ^= (c << 1);
    c.h = c.l>>1;
    c.l <<= 6;
    r.h ^= c.h;
    r.l ^= c.l;
    c.l<<=1;
    c.h>>=1;
    r.h ^= c.h;
    r.l ^= c.l;
    crc = r.i16;
}

__attribute((noinline, noclone)) void inc(uint32_t* p, uint16_t a){
    *p += a;
}

#endif

#ifdef OWSLAVE_STATES_STATES
STATE(DEVICE_COMMAND)
    switch(owslave_value){
    case OW_READ_MEM_COUNTER:
        goto OW_READ_MEM_COUNTER;
    default:
        SET_IDLE;
    }
STATE_END

OW_READ_MEM_COUNTER:
    crc = 0;
    crc16();
    RECEIVE();
    crc16();
    ((uint8_t*)&addr)[0] = owslave_value;
    RECEIVE();
    crc16();
    ((uint8_t*)&addr)[1] = owslave_value;

    do{
        do{
            SENDCRC(0xFF);
            addr++;
        }while(addr&0x1f);

        owslave_temp_counter = counter_get(0);

        for(owslave_state_temp=0; owslave_state_temp<4; owslave_state_temp++){
            SENDCRC(((uint8_t*)&owslave_temp_counter)[owslave_state_temp]);
        }
        for(owslave_state_temp=0; owslave_state_temp<4; owslave_state_temp++){
            SENDCRC(0);
        }
        crc = ~crc;
        SEND(crc&0xFF);
        SEND((crc>>8)&0xFF);
        crc = 0;
    }while(addr&0x1FF);
    SET_IDLE;

#endif
