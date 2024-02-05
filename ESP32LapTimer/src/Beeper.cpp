/*
 * This file is part of Chorus32-ESP32LapTimer 
 * (see https://github.com/AlessandroAU/Chorus32-ESP32LapTimer).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "Beeper.h"

#include "Timer.h"
#include "HardwareConfig.h"

#include <Arduino.h>

static Timer beeperTimer = Timer(50);
uint32_t melody=0b00000000000000000000000;
uint8_t melody_position=33;

void beepBits(uint32_t bits){
  melody = bits;
  melody_position=0;
  beeperTimer.reset();
}

void beep1x500(){
  beepBits(0b1111111111);
}

void beep2x500(){
  beepBits(0b1111111111001111111111);
}

void beep1x1000(){
  beepBits(0b11111111111111111111);
}

void beep1x250(){
  beepBits(0b11111);
}

void beep2x250(){
  beepBits(0b11111011111);
}

void beep3x250(){
  beepBits(0b11111011111011111);
}

void beepYes(){
  beepBits(0b11111011111);
}

void beepNo(){
  beepBits(0b00011000011);
}



void beep() {
  digitalWrite(BEEPER, HIGH);
  delay(50);
  digitalWrite(BEEPER, LOW);
  beeperTimer.reset();
}

/*void doubleBeep() {
  int i=0;
  for (i=0; i<=1; i++) {
    digitalWrite(BEEPER, HIGH);
    delay(50);
    digitalWrite(BEEPER, LOW);
    delay(50);
  }
}

void chirps() {
  int i = 0;
  for (i = 0; i <=5; i++) {
    digitalWrite(BEEPER, HIGH);
    delay(10);
    digitalWrite(BEEPER, LOW);
    delay(10);
  }
}

void fiveBeep() {
  int i=0;
  for (i=0; i<=4; i++) {
    digitalWrite(BEEPER, HIGH);
    delay(100);
    digitalWrite(BEEPER, LOW);
    delay(50);
  }
}*/

void beeperUpdate() {
  if (beeperTimer.hasTicked()) {
    if( melody_position == 33 ) return;
    if( melody_position < 32){
      if( bitRead(melody, melody_position)==1 ){
        digitalWrite(BEEPER, HIGH);
      }
      else{
        digitalWrite(BEEPER, LOW);
      }
    }
    else {
      digitalWrite(BEEPER, LOW);
    }
    melody_position++;
    beeperTimer.reset();
    //digitalWrite(BEEPER, LOW);
  }
}
