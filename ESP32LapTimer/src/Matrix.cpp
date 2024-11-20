#include <stdint.h>
#include <FastLED.h>
#include "HardwareConfig.h"
#include "Matrix.h"

#define MAX_LEDS 256        // макс. светодиодов
#define STRIP_CHIP WS2812B   // чип ленты
#define STRIP_COLOR GRB     // порядок цветов в ленте
#define STRIP_VOLT 5        // напряжение ленты, V
#define MATRIX_LENGTH 16
#define MATRIX_WIDTH 16
#define MATRIX_TYPE 1
#define MATRIX_CONNECTION 2    // 1 лента, 2 зигзаг, 3 параллел


CRGB leds[MAX_LEDS];


const uint8_t font5x7[][5] = {
  {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0 0x30 48
  {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1 0x31 49
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2 0x32 50
  {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3 0x33 51
  {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4 0x34 52
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5 0x35 53
  {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6 0x36 54
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7 0x37 55
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8 0x38 56
  {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9 0x39 57
  {0x00, 0x08, 0x08, 0x08, 0x00}, // 10 -
  {0x00, 0x00, 0x00, 0x00, 0x00}, // 11 empty
};

const uint16_t font6x12[][6] = {
  {
    0b011111111110,
    0b111111111111,
    0b110000000011,
    0b110000000011,
    0b111111111111,
    0b011111111110,
  },
  {
    0b000000000000,
    0b110000001100,
    0b111111111111,
    0b111111111111,
    0b110000000000,
    0b000000000000,
  },
  {
    0b111100000110,
    0b111110000111,
    0b110011000011,
    0b110001100011,
    0b110000111111,
    0b110000011110,
  },
  {
    0b011000000110,
    0b111000000111,
    0b110001100011,
    0b110001100011,
    0b111111111111,
    0b011110011110,
  },
  {
    0b000011111111,
    0b000011111111,
    0b000011000000,
    0b000011000000,
    0b111111111111,
    0b111111111111,
  },
  {
    0b011000111111,
    0b111000111111,
    0b110000110011,
    0b110000110011,
    0b111111110011,
    0b011111100011,
  },
  {
    0b011111111110,
    0b111111111111,
    0b110001100011,
    0b110001100011,
    0b111111100111,
    0b011111000110,
  },
  {
    0b000000000011,
    0b000000000011,
    0b111111000011,
    0b111111110011,
    0b000000111111,
    0b000000001111,
  },
  {
    0b011110011110,
    0b111111111111,
    0b110001100011,
    0b110001100011,
    0b111111111111,
    0b011110011110,
  },
  {
    0b011000111110,
    0b111001111111,
    0b110001100011,
    0b110001100011,
    0b111111111111,
    0b011111111110,
  },  

};


void startStrip() {
  FastLED.addLeds<STRIP_CHIP, STRIP_PIN, STRIP_COLOR>(leds, MAX_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(STRIP_VOLT, 500);
  FastLED.setBrightness(50);
  FastLED.show();
}

void drawDigit5x7(byte digit, int X, int Y, CRGB color) {
  for( uint8_t i=0; i<5; i++ ) {
    for( uint8_t j=0; j<7; j++) {
      if (font5x7[digit][i] & (1 << (6-j))) setPix(i + X, j + Y, color);
    }
  }
}

void drawDigit6x12(byte digit, int X, int Y, CRGB color) {
  for( uint8_t i=0; i<6; i++ ) {
    for( uint8_t j=0; j<12; j++) {
      if (font6x12[digit][i] & (1 << (11-j))) setPix(i + X, j + Y, color);
    }
  }
}


/*void drawDigit10x14(byte digit, int X, int Y, CRGB color) {
  for( uint8_t i=0; i<5; i++ ) {
    for( uint8_t j=0; j<7; j++) {
      if (font5x7[digit][i] & (1 << (6-j)) ) {
        setPix(i*2 + X, j*2 + Y, color);
        setPix(i*2 + X + 1, j*2 + Y, color);
        setPix(i*2 + X, j*2 + Y + 1, color);
        setPix(i*2 + X + 1, j*2 + Y + 1, color);
      }
    }
  }
}*/


void setPix(int x, int y, CRGB color) {
  if (y >= 0 && y < MATRIX_LENGTH && x >= 0 && x < MATRIX_WIDTH) leds[getPix(x, y)] = color;
}

uint16_t getPix(int x, int y) {
  int matrixW;
  if ( MATRIX_TYPE == 2 || MATRIX_TYPE == 4 || MATRIX_TYPE == 6 || MATRIX_TYPE == 8)  matrixW = MATRIX_LENGTH;
  else matrixW = MATRIX_WIDTH;
  int thisX, thisY;
  switch ( MATRIX_TYPE ) {
    case 1: thisX = x;                    thisY = y;                    break;
    case 2: thisX = y;                    thisY = x;                    break;
    case 3: thisX = x;                    thisY = (MATRIX_LENGTH - y - 1); break;
    case 4: thisX = (MATRIX_LENGTH - y - 1); thisY = x;                    break;
    case 5: thisX = (MATRIX_WIDTH - x - 1);  thisY = (MATRIX_LENGTH - y - 1); break;
    case 6: thisX = (MATRIX_LENGTH - y - 1); thisY = (MATRIX_WIDTH - x - 1);  break;
    case 7: thisX = (MATRIX_WIDTH - x - 1);  thisY = y;                    break;
    case 8: thisX = y;                    thisY = (MATRIX_WIDTH - x - 1);  break;
  }

  if ( !(thisY & 1) || (MATRIX_CONNECTION - 2) ) return (thisY * matrixW + thisX);   // чётная строка
  else return (thisY * matrixW + matrixW - thisX - 1);                            // нечётная строка
}

void drawMatrixDigit( uint8_t num, uint8_t pos, uint8_t max_pos){
  FastLED.clear();
    if( max_pos == 1){
        if( pos<10){
            drawDigit6x12( pos, 9, 2, CRGB::Orange );
        }
        else{
            drawDigit6x12( pos%10, 9, 2, CRGB::Orange );
            drawDigit6x12( pos/10, 1, 2, CRGB::Orange );
        }
      FastLED.show();
    }


}