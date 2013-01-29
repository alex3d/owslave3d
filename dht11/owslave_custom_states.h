#ifdef OWSLAVE_STATES_GLOBAL

#define DO_TRANSCEIVE_TO_SELF1(st, val, bm, c) do{ owslave_value=val; owslave_bitmask=bm; c; goto TRANSCEIVE; }while(0)
#define DO_TRANSCEIVE_TO_STATE1(st, val, bm, c) do{ owslave_state_label=&&st; owslave_value=val; owslave_bitmask=bm; c; goto TRANSCEIVE; }while(0)

#define SEND(val) { __label__ xxx; DO_TRANSCEIVE_TO_STATE(xxx, val, 1); xxx: ; }
#define SENDCRC(val) { __label__ xxx; DO_TRANSCEIVE_TO_STATE1(xxx, val, 1, crc16()); xxx: ; }
#define RECEIVE() SEND(0xFF)

#include "avr/pgmspace.h"
#include "avr/boot.h"
#include "avr/wdt.h"

enum{
    OW_READ_MEM_COUNTER = 0xA5,
    OW_RESET = 0xCF,

    OW_CONVERT_T=0x44,
    OW_READ_SCRATCHPAD=0xBE,
};

volatile union{
    uint8_t data[3];
    struct{
        uint8_t x[5];
        uint8_t st;
        uint8_t temp;
        uint8_t hum;
        uint8_t crc;
    };
} scratchpad;

enum DHTError
{
    DHT_ERROR_OK = 0,
    DHT_ERROR_START_FAILED_1 = 1,
    DHT_ERROR_START_FAILED_2 = 2,
    DHT_ERROR_START_FAILED_3 = 5,
    DHT_ERROR_READ_TIMEOUT = 3,
    DHT_ERROR_CHECKSUM_FAILURE = 4,
};

uint8_t readByte(uint8_t* out) 
{
    volatile uint8_t *port = &PIND;
    uint8_t bit = 1<<1;

    // Collect 8 bits from datastream, return them interpreted
    // as a byte. I.e. if 0000.0101 is sent, return decimal 5.

    uint8_t result = 0;
    uint8_t t;
    uint8_t timeout=0;
    uint8_t i;
    for (i = 8; i--; ) {
        // We enter this during the first start bit (low for 50uS) of the byte
        // Wait until pin goes high
        t = TCNT0;
        while(!(*port&bit) && !(timeout=(((uint8_t)(((uint8_t)TCNT0)-t))>200)));
        if(timeout) return 0;

        // Dataline will now stay high for 27 or 70 uS, depending on
        // whether a 0 or a 1 is being sent, respectively. Take to
        // a middle of that period to read the value
        t = TCNT0;
        _delay_us(10);
        
        while((*port&bit) && !(timeout=(((uint8_t)(((uint8_t)TCNT0)-t))>200)));
        if(timeout) return 0;

        if (((uint8_t)(((uint8_t)TCNT0)-t))>5)
            result |= 1 << i; // set subsequent bit

    }

    *out = result;
    return 1;
}

uint8_t dht_update()
{
    uint8_t _lastError = DHT_ERROR_OK;

    DDRD |= 1<<1;
    PORTD &= ~(1<<1);
    _delay_ms(20);
    DDRD &= ~(1<<1);
    _delay_us(5);
    volatile uint8_t *port = &PIND;
    uint8_t bit = 1<<1;
    
    uint8_t t;
    uint8_t timeout=0;
    t = TCNT0;
    while((*port&bit) && !(timeout=( ((uint8_t)(((uint8_t)TCNT0)-t))>100)));
    if(timeout){
        _lastError = DHT_ERROR_START_FAILED_1;
        return _lastError;
    }
    t = TCNT0;
    while(!(*port&bit) && !(timeout=( ((uint8_t)(((uint8_t)TCNT0)-t))>100)));
    if(timeout){
        _lastError = DHT_ERROR_START_FAILED_2;
        return _lastError;
    }
    t = TCNT0;
    while((*port&bit) && !(timeout=( ((uint8_t)(((uint8_t)TCNT0)-t))>100)));
    if(timeout){
        _lastError = DHT_ERROR_START_FAILED_3;
        return _lastError;
    }

    // now ready for data reception... pick up the 5 bytes coming from
    // the sensor
    uint8_t *_data = scratchpad.data;
    uint8_t i;
    for (i = 0; i < 5; i++)
        if (!readByte(_data + i))
            _lastError = DHT_ERROR_READ_TIMEOUT;

    // Restore pin to output duties
    //pinMode(_pin, OUTPUT); // was: DDRC |= _BV(dht_PIN);
    //digitalWrite(_pin, HIGH); // was: PORTC |= _BV(dht_PIN);

    if (!_lastError) {
        // See if data received consistent with checksum received
        uint8_t checkSum = _data[0] + _data[1] + _data[2] + _data[3];
        if (_data[4] != checkSum)
            _lastError = DHT_ERROR_CHECKSUM_FAILURE;
        //else{
            scratchpad.temp = _data[0];
            scratchpad.hum = _data[2];
        //}
    }

    return _lastError;
}

inline void states_init(void){
        scratchpad.crc = 0xB5;
}

volatile uint8_t convert_flag = 0;

inline void owslave_idle(void){
    if(convert_flag){
        convert_flag = 0;
        scratchpad.crc++;
        scratchpad.temp=0;
        scratchpad.hum=0;
        scratchpad.st = dht_update();

    }
/*
        PORTD |= (1<<5);
        _delay_ms(300);
        PORTD &= ~(1<<5);
        _delay_ms(300);
*/
}

#endif

#ifdef OWSLAVE_STATES_STATES
STATE(DEVICE_COMMAND)
    switch(owslave_value){
    case OW_RESET:
        wdt_enable(WDTO_15MS);
        while(1);
    case OW_CONVERT_T:
        convert_flag = 1;
        SET_IDLE;
    case OW_READ_SCRATCHPAD:
        goto OW_READ_SCRATCHPAD;
    default:
        SET_IDLE;
    }
STATE_END

OW_READ_SCRATCHPAD:
    for(owslave_state_temp=0; owslave_state_temp<5+4; owslave_state_temp++)
        SEND(scratchpad.data[owslave_state_temp]);
    SET_IDLE;

#endif
