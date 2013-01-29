#ifdef OWSLAVE_STATES_GLOBAL

__attribute__((section("vectors")))  __attribute__((naked)) __attribute__((used))
void start(){
    asm("rjmp 0x600");
}


enum{
    OW_SEARCH_ROM=0xF0,
    OW_READ_ROM=0x33,
    OW_MATCH_ROM=0x55,
    OW_SKIP_ROM=0xCC,
    OW_ALARM_SEARCH=0xEC,
};


#define BEGIN_STATES do{ goto *owslave_state_label;
#define END_STATES }while(0);
#define DO_RECEIVE_TO_STATE(st) do{ owslave_state_label=&&st; owslave_value=0xFF; owslave_bitmask=1; goto TRANSCEIVE; }while(0)
#define DO_RECEIVE_TO_SELF(st) do{ owslave_value=0xFF; owslave_bitmask=1; goto TRANSCEIVE; }while(0)
#define DO_TRANSCEIVE_TO_SELF(st, val, bm) do{ owslave_value=val; owslave_bitmask=bm; goto TRANSCEIVE; }while(0)
#define DO_TRANSCEIVE_TO_STATE(st, val, bm) do{ owslave_state_label=&&st; owslave_value=val; owslave_bitmask=bm; goto TRANSCEIVE; }while(0)
#define SET_STATE(st) owslave_state_label=&&st
#define SET_IDLE goto IDLE

#define SEND(val) { __label__ xxx; DO_TRANSCEIVE_TO_STATE(xxx, val, 1); xxx: ; }
#define RECEIVE() SEND(0xFF)


#include "avr/pgmspace.h"
#include "avr/boot.h"
#include "avr/wdt.h"

enum{
    OW3D_READ_MEM = 0xA5,
};

inline void states_init(void){
    TCCR1B = 5; //2313
    TCNT1 = 0;
}


inline void owslave_idle(void){
    asm(
    "in  r0, 0x38 \n"
    "sbrc    r0, 7 \n"
    "rjmp start+0x26 \n"
    );
    
    
    /*    
    if(TIFR&(1<<TOV1)){
        //asm("rjmp 0x32");
        asm("rjmp start+0x26");
        //asm("rjmp 0x32");
    }*/
    
    
}

#undef OWSLAVE_STATES_GLOBAL
#endif

#ifdef OWSLAVE_STATES_STATES

ROM_COMMAND:
    if(owslave_value!=OW_SEARCH_ROM) SET_IDLE;

   
    addru.i16 = UID;
    do{
        //owslave_state_temp1 = pgm_read_byte(addru.i16++);
        for(owslave_state_temp=1; owslave_state_temp; owslave_state_temp<<=1){
            //if( owslave_state_temp1 & owslave_state_temp )
            if( pgm_read_byte(addru.i16) & owslave_state_temp )
                DO_TRANSCEIVE_TO_STATE(XXX, 0xB8, 0x20);
            else
                DO_TRANSCEIVE_TO_STATE(XXX, 0xC0, 0x20);
            XXX:
            //if( ((owslave_value>>4)^owslave_value)&(1<<3) ) // >>4 faster than <<3 on AVR
            if( (__builtin_avr_swap(owslave_value)^owslave_value)&(1<<3) ) // >>4 faster than <<3 on AVR
                 SET_IDLE;
        }
        addru.i16++;
    //}while(!((addru.h)&0x20) );
    }while(addru.i16 < UID+8 );
    

    RECEIVE();

    if(owslave_value!=OW3D_READ_MEM) SET_IDLE;


    RECEIVE();
    addru.l = owslave_value;
    RECEIVE();
    addru.h = owslave_value;

    if(addru.h&0x80){
        addru.h &= 0x7F;
        do{
            if(( addru.h & 0xF0)==0x10){
                SEND(boot_signature_byte_get( addru.i16  ));
            }else if(( addru.h & 0xF0)==0x20){
                SEND(boot_lock_fuse_bits_get( addru.i16  ));
            }else{
                SEND( pgm_read_byte(addru.i16) );
            }
            addru.i16++;
        }while( addru.l&0x7f );
    }else{
        addru.l &= ~(0x1f);
        if(addru.h>=0x6) SET_IDLE; // prevent bootloader overwrite
                        
        do{
            RECEIVE();
            owslave_state_temp = owslave_value;
            RECEIVE();
            union{uint16_t i16; struct{uint8_t l, h;}; } word;
            word.l = owslave_state_temp;
            word.h = owslave_value;

            if(addru.i16==0){
                word.i16 = 0xC2FF; // jump to bootloader
            }

            //boot_page_fill_safe(addru.i16, word.i16);
            boot_page_fill(addru.i16, word.i16);
            addru.i16 +=2;
        }while(addru.l&0x1f);

        addru.i16 -= 32;

        SEND(OW3D_READ_MEM);

        //boot_page_erase_safe(addru.i16);
        //boot_page_write_safe(addru.i16);

        boot_page_erase(addru.i16);
        //boot_spm_busy_wait();
        boot_page_write(addru.i16);
        //boot_spm_busy_wait();

        //boot_rww_enable_safe();
        
        
    }

    TCNT1 = 0; // reset app timer

    SET_IDLE;

#undef OWSLAVE_STATES_STATES
#endif
