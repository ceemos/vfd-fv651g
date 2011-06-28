#include <stdint.h>

#define BUFFLEN 10
#define USE_SCROLL 0

// Send 16bit-Anode Data instead of 8bit-character to the Display.
#define RAWMODE 1

void inittxt(void);
uint16_t getcharat(char pos);
void resetText();
void writeChar(uint8_t c);

#if RAWMODE
typedef uint16_t character;
#else
typedef uint8_t character;
#endif

extern volatile uint8_t currentPosition;
extern volatile character text[BUFFLEN];
extern volatile uint8_t special;