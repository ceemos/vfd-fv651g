#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <stdint.h>

#define BUFFLEN 16

//Buffer for incoming text
volatile uint16_t text[BUFFLEN];

volatile uint16_t scrollpos;

//Initial text
char inittext[] PROGMEM="Habe nun, ach! Philosophie,    Juristerei und Medizin,     Und leider auch Theologie     Durchaus studiert, mit heißem Bemühn.    Da steh' ich nun, ich armer Tor,        Und bin so klug als wie zuvor!      Heiße Magister, heiße Doktor gar,       Und ziehe schon an die zehen Jahr'      Herauf, herab und quer und krumm     Meine Schüler an der Nase herum -    Und sehe, daß wir nichts wissen können!   Das will mir schier das Herz verbrennen.     Zwar bin ich gescheiter als alle die Laffen,       Doktoren, Magister, Schreiber und Pfaffen;    Mich plagen keine Skrupel noch Zweifel,    Fürchte mich weder vor Hölle noch Teufel -    Dafür ist mir auch alle Freud' entrissen,     Bilde mir nicht ein, was Rechts zu wissen,    Bilde mir nicht ein, ich könnte was lehren,     Die Menschen zu bessern und zu bekehren.    Auch hab' ich weder Gut noch Geld,    Noch Ehr' und Herrlichkeit der Welt;   Es möchte kein Hund so länger leben!   Drum hab' ich mich der Magie ergeben,   Ob mir durch Geistes Kraft und Mund    Nicht manch Geheimnis würde kund;    Daß ich nicht mehr mit sauerm Schweiß   Zu sagen brauche, was ich nicht weiß;    Daß ich erkenne, was die Welt   Im Innersten zusammenhält,   Schau' alle Wirkenskraft und Samen,   Und tu' nicht mehr in Worten kramen.         ";

uint16_t font[] PROGMEM={
    0b00000000000000, //sp
    0b0001000001000000, //!
    0b0000000001001000, //""
    0b0100101101010000, //#
    0b1000111001010100, //$
    0b00000000000000, //%
    0b00000000000000, //&
    0b0000000001000000, //'
    0b0000010000100000, //(
    0b0001000010000000, //)
    0b0001111111100000, //*
    0b0000101101000000, //+
    0b0001000000000000, //,
    0b0000001100000000, //-
    0b0000010000000000, //.
    0b0001000000100000, ///
    0b1111000000111100, //0
    0b0010000000001000, //1
    0b1100001100001100, //2
    0b1010001100001100, //3
    0b0010001100011000, //4
    0b1010001100010100, //0b1000011000010100, //5
    0b1110001100010100, //6
    0b0010000100001100, //7
    0b1110001100011100, //8
    0b1010001100011100, //9
    0b0000010100000000, //:
    0b0001001000000000, //;
    0b1000010000000000, //<
    0b1000001100000000, //=
    0b1001000000000000, //>
    0b0000010000100100, //?
    0b1110011000011100, //@
    0b0110001100011100, //A
    0b1010100101001100, //B
    0b1100000000010100, //C
    0b1010100001001100, //D
    0b1100001000010100, //E
    0b0100001000010100, //F
    0b1110100100010100, //G
    0b0110001100011000, //H
    0b1000100001000100, //I
    0b1010000000001000, //J
    0b0100011000110000, //K
    0b1100000000010000, //L
    0b0110000010111000, //M
    0b0110010010011000, //N
    0b1110000000011100, //O
    0b0100001100011100, //P
    0b1110010000011100, //Q
    0b0100011100011100, //R
    0b1000011000010100, //0b1010000110000100, //S
    0b0000100001000100, //T
    0b1110000000011000, //U
    0b0101000000110000, //V
    0b0111010000011000, //W
    0b0001010010100000, //X
    0b0000100010100000, //Y
    0b1001000000100100, //Z
    0b1100000000010100, //[
    0b0000010010000000, //backslash 
    0b1010000000001100, //]
    0b0000000000000100, //^
    0b1000000000000000, //_
    0b0000000010000000, //`
    0b1100011000000000, //A
    0b1100011000010000, //B
    0b1100001100000000, //C
    0b1011000100001000, //D
    0b1101001000000000, //E
    0b0000101100100000, //F
    0b1010100100000000, //G
    0b0110001100010000, //H
    0b0000100000100000, //I
    0b1010000000000000, //J
    0b0000110101000000, //K
    0b0100000000010000, //L
    0b0110101100000000, //M
    0b0110001100000000, //N
    0b1110001100000000, //O
    0b0101001000000000, //P
    0b1110011100000000, //Q
    0b0100001000000000, //R
    0b1000010100000000, //S
    0b0000101100000000, //T
    0b1110000000000000, //U
    0b0010010000000000, //V
    0b0111010000000000, //W
    0b0001011100000000, //X
    0b1010010000000000, //Y
    0b1001001000000000, //Z
    0b0000101001000000, //{
    0b0000100001000000, //|
    0b0000100101000000, //}
    0b0000001100000000, //~
    0b1111111111111111, //(del)
};

//Routine to init the text subsystem
void inittxt(void) {
    int t=0;
    char c;
//     int seg = 1;
    
    do {
	c=pgm_read_byte(&inittext[t]);
	text[t]=pgm_read_word(&font[(int)(c-32)]);
	t++;
	//seg <<= 1;
	//text[t++]=0x0F0F;
    } while (t<16);

//     int t=0;
//     char c;
//     //Copypaste inittext to text buffer
//     do {
// 	c=pgm_read_byte(&inittext[t]);
// 	text[t++]=pgm_read_word(&font[(int)(c-32)]);
//     } while (c!=0);

}

//Returns the character that should be on the pos't digit position of the VFD
//according to the latest scrollpos- and text[]-contents.
//Includes an overlap of OVERLAP spaces between end and begin of scrolling text
uint16_t getcharat(char pos) {
    uint8_t c = pgm_read_byte(&inittext[scrollpos + pos]);
    if(!c){
      scrollpos = 0;
      c = 32;
    }
    return pgm_read_word(&font[(int)(c-32)]);
    //return text[(int) pos];
}

//Timer interrupt which advances the scroll position by one.
ISR(TIMER1_COMPA_vect) {
    scrollpos++;
}

//Uart receive interrupt
ISR(USART_RX_vect) { 
//     static unsigned int wpos=BUFFLEN;
//     unsigned char c;
//     c=UDR;
//     //Black text-handling magic ^_^
//     if (c<32 && wpos==0) return;
//     if (wpos==0) scrollpos=0;
//     text[wpos]=c;
//     if (wpos<BUFFLEN) wpos++;
//     textlen=wpos;
//     if (c==13 || c==10 || c==12) wpos=0;
    // </magic>
}



