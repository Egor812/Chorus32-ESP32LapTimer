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
#include "Laptime.h"

#include <stdint.h>
#include <Arduino.h>

#include "HardwareConfig.h"
#include "settings_eeprom.h"
#include "Comms.h"
#include "Beeper.h"
#include "Matrix.h"

static volatile uint32_t LapTimes[MAX_NUM_RECEIVERS][MAX_LAPS_NUM]; // absolute time in millis
uint32_t bestLap=0xFFFFFFFF;
static volatile int lap_counter[MAX_NUM_RECEIVERS] = {0, 0, 0, 0, 0, 0}; //Keep track of what lap we are up too
static int last_lap_sent[MAX_NUM_RECEIVERS];

static uint32_t MinLapTime = 5000;  //this is in millis
static uint32_t start_time = 0;

void resetLaptimes() {
  for (int i = 0; i < getNumReceivers(); ++i) {
    lap_counter[i] = 0;
    last_lap_sent[i] = 0;
  }
}

void sendNewLaps() {
  for (int i = 0; i < getNumReceivers(); ++i) {
    int laps_to_send = lap_counter[i] - last_lap_sent[i];
    if(laps_to_send > 0) {
      for(int j = 0; j < laps_to_send; ++j) {
        sendLap(lap_counter[i] - j, i); //comms
      }
      last_lap_sent[i] += laps_to_send;
    }
  }
}

uint32_t getLaptime(uint8_t receiver, uint8_t lap) {
  return LapTimes[receiver][lap];
}

uint32_t getLaptime(uint8_t receiver) {
  return getLaptime(receiver, lap_counter[receiver]);
}

uint32_t getLaptimeRel(uint8_t receiver, uint8_t lap) {
  if(lap == 1) {
    return getLaptime(receiver, lap) - start_time;
  } else if(lap == 0) {
    return 0;
  }
  return getLaptime(receiver, lap) - getLaptime(receiver, lap - 1);
}

uint32_t getLaptimeRelToStart(uint8_t receiver, uint8_t lap) {
  return getLaptime(receiver, lap) - start_time;
}

uint32_t getLaptimeRel(uint8_t receiver) {
  return getLaptimeRel(receiver, lap_counter[receiver]);
}

/// @brief 
/// @param receiver 0..5
/// @param time absolute in millis
/// @return 
uint8_t addLap(uint8_t receiver, uint32_t time) {
  uint32_t cur;
  uint32_t prev;

  cur = time-LapTimes[ receiver ][ lap_counter[receiver] ];
  if( lap_counter[receiver]>0 ) prev = LapTimes[receiver][lap_counter[receiver]]-LapTimes[receiver][lap_counter[receiver]-1];
  else prev = cur;
  if( cur<bestLap ) bestLap=cur;

  lap_counter[receiver] = lap_counter[receiver] + 1;
  LapTimes[receiver][lap_counter[receiver]] = time;

  beepLap(cur, prev, bestLap);

  #ifdef LED_MATRIX
    //При старте гонки после прохождения кольца - до старта на табло 0 (isShouldWaitForFirstLap==0)
    //При старте по таймеру - до старта на табло 1 (isShouldWaitForFirstLap==1)
    displayLap(receiver, lap_counter[receiver] + isShouldWaitForFirstLap() );
  #endif

  return lap_counter[receiver];
}

void beepLap( uint32_t time, uint32_t prev, uint32_t best )
{
  if( isSoundEnabled() ) {
    if( time==best) beep3x250();
    else{
      if( time<prev) beep2x250();
      else beep1x250();
    }
  }
}

void displayLap( uint8_t pilot_num, uint8_t lap){
  drawMatrixDigit(pilot_num, lap, getNumReceivers() );
}

uint32_t getMinLapTime() {
  return MinLapTime;
}

void setMinLapTime(uint32_t time) {
  MinLapTime = time;
}

uint8_t getCurrentLap(uint8_t receiver) {
  return lap_counter[receiver];
}

void startRaceLap() {
  resetLaptimes();
  #ifdef LED_MATRIX
    //При старте гонки после прохождения кольца - до старта на табло 0
    //При старте по таймеру - до старта на табло 1
    displayLap(0, isShouldWaitForFirstLap() );
  #endif
  start_time = millis();
}
