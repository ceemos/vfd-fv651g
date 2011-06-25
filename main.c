//This program is licensed under the GPLv3; see license.txt for more info.
//Copyright 2008 Sprite_tm

#include "io.h"
#include "uart.h"

#include "usbconfig.h"
#include "usbdrv/usbdrv.h"
#include "requests.h"


#include "avr/interrupt.h"
#include "avr/pgmspace.h"
#include "util/delay.h"


#include <stdint.h>

static uint8_t currentPosition, bytesRemaining;

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    if(rq->bRequest == CUSTOM_RQ_SET_TEXT){
        currentPosition = 0;                // initialize position index
        bytesRemaining = rq->wLength.word;  // store the amount of data requested
        if(bytesRemaining > BUFFLEN) // limit to buffer size
            bytesRemaining = BUFFLEN;
        return USB_NO_MSG;        // tell driver to use usbFunctionWrite()
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

// Daten Verarbeiten
uchar usbFunctionWrite(uchar *data, uchar len)
{
    uint8_t i;
    if(len > bytesRemaining)                // if this is the last incomplete chunk
        len = bytesRemaining;               // limit to the amount we can store
    bytesRemaining -= len;
    for(i = 0; i < len; i++)
        text[currentPosition++] = data[i];
    return bytesRemaining == 0;             // return 1 if we have all data
}

//Main routine
int main(void) {
    int d;
    uint16_t v;
    
    cli();
    initio();
    inittxt();

    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    uint8_t i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        _delay_ms(1);
    }
    usbDeviceConnect();
    
    sei();

    //Go display stuff
    while(1) {
	for (d=0; d<11; d++) {
	    v=getcharat(d);
	    setvfd(d,v);
	    usbPoll();
	    _delay_ms(1);
	}
    }
}
