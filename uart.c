#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <stdint.h>

#include "uart.h"

#if !RAWMODE
#include "chartable.h"
#endif

//Buffer for incoming text
volatile character text[BUFFLEN];

volatile uint8_t currentPosition;

#if USE_SCROLL
volatile uint16_t scrollpos;
#endif
// Value of Digit 1G (the symbols)
volatile uint8_t special;

#if RAWMODE
void inittxt(void) {
    int i;
    for(i = 0; i < BUFFLEN; i++){
      text[i] = 0;
    }
    // Intial Text - it's not as simple as it was in non-Rawmode
    text[0] = 0b0100001000010100; // F
    text[1] = 0b0101000000110000; // V
    text[2] = 0b1110001100010100; // 6
    text[3] = 0b1010001100010100; // 5
    text[4] = 0b0010000000001000; // 1
    text[5] = 0b1110100100010100; // G
    
    
    special = 0b00; 

}
#else
//Initial text
char inittext[] PROGMEM="FV651g";

//Routine to init the text subsystem
void inittxt(void) {
    int t=0;
    char c;
    //Copypaste inittext to text buffer
    do {
	c=pgm_read_byte(&inittext[t]);
	text[t++]=c;
    } while (c!=0);
    
    special = 0b100; // dcc Symbol

}
#endif

inline void resetText(){
  currentPosition = 0;
}

#if RAWMODE
inline void writeChar(uint8_t c){
  ((uint8_t*) text)[currentPosition++] = c;
}
#else
inline void writeChar(uint8_t c){
  text[currentPosition++] = c;
}
#endif

//Returns the character that should be on the pos't digit position of the VFD
//according to the latest scrollpos- and text[]-contents.
//Includes an overlap of OVERLAP spaces between end and begin of scrolling text
uint16_t getcharat(char pos) {
  if(pos == 10){
    return special << 8;
  }
#if RAWMODE
  return text[(int) pos];
#else
#if USE_SCROLL  
  uint8_t c = text[pos + scrollpos];
  if(!c){
    scrollpos = 0;
    c = 32;
  }
#else 
  uint8_t c = text[pos];
  if(!c) c = 32;
#endif
  return pgm_read_word(&font[(int)(c-32)]);
#endif
  //return text[(int) pos];
}

#if USE_SCROLL
//Timer interrupt which advances the scroll position by one.
ISR(TIMER1_COMPA_vect) {
    scrollpos++;
}
#endif



