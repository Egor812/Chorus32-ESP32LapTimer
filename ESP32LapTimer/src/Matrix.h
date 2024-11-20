#include <stdint.h>
#include <FastLED.h>

void startStrip();

void drawMatrixDigit( uint8_t num, uint8_t pos, uint8_t max_pos);
uint16_t getPix(int x, int y);
void setPix(int x, int y, CRGB color);
void drawDigit5x7(byte digit, int X, int Y, CRGB color);
void drawDigit6x12(byte digit, int X, int Y, CRGB color);
void drawDigit10x14(byte digit, int X, int Y, CRGB color);