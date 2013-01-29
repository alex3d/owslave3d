//#define USE_PTR
//#define REGISTER
//#define VOLATILE
#define NO_INTERRUPTS
//#define EXCLUSIVE_TRANSCEIVE
#define USE_COMPUTED_GOTO 1

// Clock is set via fuse, at least to 4MHz
#define F_CPU   8000000

#if F_CPU>4000000
#define PRESCALE 64
#else
#define PRESCALE 8
#endif

// at least "c" microseconds
#define T_(c) ((F_CPU/PRESCALE+1000000/c-1)/(1000000/c))
#define T_PRESENCE T_(180)
#define T_PRESENCEWAIT (T_(35))
#define T_RESET T_(400)
#define T_SAMPLE ((T_(15)*13+9)/10 + 1)
// +30% for internal clock accuracy, +1 for unpredictable prescaler
//#define T_SAMPLE 1

#define TICKS_IN_SAMPLE ((15*F_CPU+1000000-1)/1000000)

#if T_RESET>250
#error T_RESET too long
#endif

#ifdef PRINT_TIMINGS
#define VAR_NAME_VALUE(var) exec bc print #var, " = ", var, " ticks, ", (var)*PRESCALE*1000000/F_CPU, " us \n"
VAR_NAME_VALUE(T_SAMPLE)
VAR_NAME_VALUE(T_RESET)
VAR_NAME_VALUE(T_PRESENCEWAIT)
VAR_NAME_VALUE(T_PRESENCE)
exec bc print "TICKS_IN_SAMPLE = ", TICKS_IN_SAMPLE, " ticks\n"
#endif

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<avr/pgmspace.h>

#define SIGRD 5

#define inline __attribute((always_inline))

/* 168
#define GIMSK EIMSK
#define GIFR EIFR
#define TIFR TIFR0
#define TIMSK TIMSK0
#define TCCR0 TCCR0B
#define MCUCR EICRA
*/
#ifndef GIFR
#define GIFR EIFR // attiny2313
#endif
inline void owslave_init(void){

//	TCCR0A = 0; 168
#if PRESCALE==64
//    TCCR0 = 0x03;           // Prescaler 1/64
    TCCR0B = 3;
#elif PRESCALE==8
//    TCCR0 = 0x02;           // Prescaler 1/8
    TCCR0B = 2;
#else
    #error unknown PRESCALER
#endif
    MCUCR |= (2 << ISC00);  // Interrupt on falling
    TIMSK |= (1 << TOIE0);  // unmask timer
    GIMSK |= (1 << INT0);   // unmask owpin

    // INT0
    PORTD &= ~(1<<2);
    DDRD &= ~(1<<2);
}


static inline void owslave_mask_pin(void) {
    GIMSK &= ~(1 << INT0);
}

static inline void owslave_unmask_pin(void) {
    GIFR = (1 << INTF0);
    GIMSK |= (1 << INT0);
}

void inline owslave_set_timeout(uint8_t timeout){
    TCNT0 = -timeout;
    //OCR0A = TCNT0 + timeout; 

}

static inline void owslave_pin_low(void) { DDRD |= (1<<2); }
static inline void owslave_pin_hiz(void) { DDRD &= ~(1<<2); }
#define owslave_pin_value() (PIND & (1<<2))

static inline void owslave_pin_falling(void){ MCUCR = (MCUCR&~(3<<ISC00))|(2 << ISC00);  /* Interrupt on falling */ }
static inline void owslave_pin_rising(void) { MCUCR = (MCUCR&~(3<<ISC00))|(3 << ISC00);  /* Interrupt on rising  */ }
#define owslave_pin_int() (GIFR&(1<<INTF0))
#define owslave_pin_clear_int() do{GIFR = 1 << INTF0;}while(0)
#define owslave_timer_int() (TIFR&(1<<TOV0))
#define owslave_timer_clear_int() do{TIFR = 1 << TOV0;}while(0)

static inline void alive(void){
    static uint8_t val=0;
    val++;
    if(val&1)
        PORTD |= (1<<5);
    else
        PORTD &= ~(1<<5);
}
#ifndef VOLATILE
#define volatile
#else
#pragma message( "\033[1m" "VOLATILE" "\033[0m")
#endif
#ifdef REGISTER
#pragma message( "\033[1m" "REGISTER" "\033[0m")
// safe to use r2-r7
volatile register uint8_t owslave_state asm("r2");
volatile register uint8_t owslave_bitmask asm("r3");
volatile register uint8_t owslave_value asm("r4");
volatile register uint8_t owslave_state_temp asm("r5");
volatile register void* owslave_state_label asm("r6");
#else
volatile uint8_t owslave_state;
volatile uint8_t owslave_bitmask;
volatile uint8_t owslave_value;
volatile uint8_t owslave_state_temp;
volatile void* owslave_state_label;
#endif
#undef volatile

uint8_t* owslave_state_temp_ptr;

//static const uint8_t UID[8]={
__attribute((section(".UID"))) const uint8_t UID[8]={
//PROGMEM const uint8_t UID[8]={
//__flash const uint8_t UID[8]={
//0x28, 0x24, 0x86, 0x0C, 0x03, 0x00, 0x00, 0xD5
0x1D, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x8B
};
//#define owslave_uid_byte(i) UID[i]
#define owslave_uid_byte(i) pgm_read_byte(UID+i)


#define owslave_timer_interrupt ISR(TIMER0_OVF_vect)
#define owslave_pin_interrupt ISR(INT0_vect)
