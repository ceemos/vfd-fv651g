#include <stdint.h>

#define BUFFLEN 16

void inittxt(void);
uint16_t getcharat(char pos);
extern volatile uint8_t text[BUFFLEN];