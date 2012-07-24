#ifdef OWSLAVE_STATES_GLOBAL

#include "crc8.h"

#if !USE_COMPUTED_GOTO
enum{
    DEVICE_COMMAND=STATE_FIRST_CUSTOM,
    READ_SCRATCHPAD,
};
#endif
enum{
    // DS18B20
    OW_CONVERT_T=0x44,
    OW_WRITE_SCRATCHPAD=0x4E,
    OW_READ_SCRATCHPAD=0xBE,
    OW_COPY_SCRATCHPAD=0x48,
    OW_RECALL_E2=0xB8,
    OW_READ_POWER_SUPPLY=0xB4,
};

union{
    uint8_t data[9];
    struct{
        uint16_t temp;
        uint8_t th;
        uint8_t tl;
        uint8_t config;
        uint8_t xFF;
        uint8_t reserved;
        uint8_t x10;
        uint8_t crc;
    };
} scratchpad;
volatile uint8_t sample_flag;

inline void states_init(void){
    // init scratchpad
    scratchpad.temp=0;
    scratchpad.x10 = 0x10;
    scratchpad.xFF = 0xFF;
    scratchpad.config = 0x7F;

    scratchpad.crc = crc8(scratchpad.data,8);
}

inline void owslave_idle(void){
    if(sample_flag){
        sample_flag=0;
        if(scratchpad.temp<=2000-100)
            scratchpad.temp+=100;
        else
            scratchpad.temp=0;
        scratchpad.crc = crc8(scratchpad.data,8);
    }
}
#endif

#ifdef OWSLAVE_STATES_STATES
STATE(DEVICE_COMMAND)
    switch(owslave_value){
    case OW_CONVERT_T:
        sample_flag=1;
        SET_IDLE;
    case OW_READ_SCRATCHPAD:
        goto READ_SCRATCHPAD_INIT;
    default:
        SET_IDLE;
    }
STATE_END

STATE(READ_SCRATCHPAD)
#ifdef USE_PTR
    if((owslave_state_temp_ptr)>=scratchpad.data+9)
        SET_IDLE;
#else
    if(owslave_state_temp>=9)
        SET_IDLE;
#endif
READ_SCRATCHPAD_1:
#ifdef USE_PTR
    DO_TRANSCEIVE_TO_SELF(READ_SCRATCHPAD, *(owslave_state_temp_ptr++), 1);
#else
    DO_TRANSCEIVE_TO_SELF(READ_SCRATCHPAD, scratchpad.data[owslave_state_temp++], 1);
#endif

READ_SCRATCHPAD_INIT:
#ifdef USE_PTR
    owslave_state_temp_ptr=scratchpad.data;
#else
    owslave_state_temp=0;
#endif
    SET_STATE(READ_SCRATCHPAD);
    goto READ_SCRATCHPAD_1;
STATE_END
#endif
