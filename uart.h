#include <stdint.h>

#define BUFFLEN 16
#define USE_SCROLL 0

void inittxt(void);
uint16_t getcharat(char pos);
void resetText();
void writeChar(char c);

extern volatile uint8_t currentPosition;
extern volatile uint8_t text[BUFFLEN];