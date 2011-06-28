//This program is licensed under the GPLv3; see license.txt for more info.
//Copyright 2008 Sprite_tm

#include "io.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#  include <avr/iotn2313.h>
#include <stdint.h>

//Remember: All signals are inverted due to the transistor-based
//5V->12V-conversion.
#define SHIFT_DATA 	PD3
#define SHIFT_STROBE	PD4
#define SHIFT_CLK	PD5

#define PWM_OUT		PB2

#define USE_PWM 1


//Delay, in uS, to minimally make a pulse to the shiftregisters last
#define DELAY 5

//Init all io-ports
void initio(void) {
    //Init I/O
    DDRD=(1<<SHIFT_DATA)|(1<<SHIFT_CLK)|(1<<SHIFT_STROBE);
    PORTD=0;
#if USE_SCROLL
    TCCR1A=0;
    TCCR1B=(1<<WGM12)|(1<<CS12)|(1<<CS10);
    //Timer1 ticks at 11KHz.
    OCR1A=11000/3; //1/3 second
    TIMSK=(1<<OCIE1A);
#endif
    
#if USE_PWM
    DDRB=(1<<PWM_OUT);
    PORTB=0;

    TCCR0A=(1<<COM0A1)|(1<<WGM01)|(1<<WGM00); //fast pwm on ocr0a
    TCCR0B=(1<<CS00);
    OCR0A=10;

    ACSR=(1<<ACBG); //enable AIN1<->vref-comparison
    DIDR=(1<<AIN1D); //disable AIN1 digital pin
#endif
}

//Boostconverter adjustments. Check analog comparator, adjust pwm-value on it.
#define STEPUP 1
#define STEPDOWN 1
inline void handlepwm(void) {
#if USE_PWM
    int t=OCR0A;
    if (ACSR&(1<<ACO)) {
	if (t<150) t+=STEPUP;
    } else {
	if (t>STEPDOWN) t-=STEPDOWN;
    }
    OCR0A=t;
#endif
}

//Shift one bit into the shiftregister chain.
void shift(char bit) {
    if (bit) {
	PORTD&=~(1<<SHIFT_DATA);
    } else {
	PORTD|=(1<<SHIFT_DATA);
    }
    _delay_us(DELAY);
    PORTD&=~(1<<SHIFT_CLK);
    _delay_us(DELAY);
    PORTD|=(1<<SHIFT_CLK);
}

//Pulse strobe of the shiftregister-chain
void strobe(void) {
    PORTD&=~(1<<SHIFT_STROBE);
    _delay_us(DELAY);
    PORTD|=(1<<SHIFT_STROBE);
}

//Activate the grate of the 'digit'th digit and set the segments
//to the value as described in 'data'
void setvfd(char digit, uint16_t data) {
    char x;
    uint16_t b;
    
    shift(0); // 2 Excessive Bits at the right End
    shift(0);
    
    shift(0); // P17: Colon/(digital): not used
    
    //Second: Send the digit data
    b=0x8000;
    for (x=0; x<16; x++) {
	if (data & b) {
	    shift(1);
	} else {
	    shift(0);
	}
	b /=2; // >>=1
    }

    shift(0); // two not used Pins (22/23)
    shift(0);
    
    //Third: send the grate data
    //All zeroes but the one digit we want selected.
    for (x=10; x>=0; x--) {
	if (x==digit) {
	    shift(1); 
	} else {
	    shift(0);
	}
    }
    
    //Finally: strobe output through.
    strobe();
}
