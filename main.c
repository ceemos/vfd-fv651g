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

static uint8_t bytesRemaining;

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    /* static uchar    dataBuffer[4];  //buffer must stay valid when usbFunctionSetup returns 

    if(rq->bRequest == CUSTOM_RQ_ECHO){ // echo -- used for reliability tests 
        dataBuffer[0] = rq->wValue.bytes[0];
        dataBuffer[1] = rq->wValue.bytes[1];
        dataBuffer[2] = rq->wIndex.bytes[0];
        dataBuffer[3] = rq->wIndex.bytes[1];
        usbMsgPtr = dataBuffer;         // tell the driver which data to return 
        return 4;
    }else */ if(rq->bRequest == CUSTOM_RQ_SET_TEXT){
        resetText();                // initialize position index
        bytesRemaining = rq->wLength.word;  // store the amount of data requested
        if(bytesRemaining > sizeof(text)) // limit to buffer size
            bytesRemaining = sizeof(text);
        return USB_NO_MSG;        // tell driver to use usbFunctionWrite()
    } /*else if(rq->bRequest == CUSTOM_RQ_CLEAR){
      resetText();
      writeChar(0);
      return 0;
    }*/ else if(rq->bRequest == CUSTOM_RQ_SYMBOL){
      special = rq->wValue.bytes[0];
      return 0;
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
        writeChar(data[i]);
    return bytesRemaining == 0;             // return 1 if we have all data
}

//Main routine
int main(void) {
    int d;
    uint16_t v;
    
    // cli();
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
	    _delay_ms(1);
	}
	usbPoll();
	handlepwm();
    }
}
