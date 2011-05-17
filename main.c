//This program is licensed under the GPLv3; see license.txt for more info.
//Copyright 2008 Sprite_tm

#include "io.h"
#include "avr/interrupt.h"
#include "avr/pgmspace.h"
#include "util/delay.h"
#include "uart.h"

#include <stdint.h>

//Main routine
int main(void) {
    int d;
    uint16_t v;
    initio();
    inittxt();

    //Go display stuff
    while(1) {
	for (d=0; d<11; d++) {
	    v=getcharat(d);
	    setvfd(d,v);
	    _delay_ms(1);
	}
    }
}
