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
#include "Calibration.h"

#include "ADC.h"
#include "HardwareConfig.h"
#include "RX5808.h"
#include "settings_eeprom.h"
#include "OLED.h"
#include "Timer.h"
#include "Utils.h"

static int calibrationFreqIndex = 0;
static bool isCurrentlyCalibrating = false;
static Timer calibrationTimer = Timer(50);
static uint16_t maxFreq = 0;

bool isCalibrating() {
  return isCurrentlyCalibrating;
}

void rssiCalibration() {
  for (uint8_t i = 0; i < getNumReceivers(); i++) {
    EepromSettings.RxCalibrationMin[i] = 5000;
    EepromSettings.RxCalibrationMax[i] = 0;
  }
  isCurrentlyCalibrating = true;
  calibrationFreqIndex = 0;
  setModuleFrequencyAll(channelFreqTable[calibrationFreqIndex]);
  setRXADCfilter(LPF_10Hz);
  calibrationTimer.reset();
  Serial.println("Calibration started:");
  setDisplayScreenNumber(2);
}


void rssiCalibrationReset(){
  for (int i = 0; i < getNumReceivers(); i++) {
    EepromSettings.RxCalibrationMax[i] = RSSI_ADC_READING_MAX;
    EepromSettings.RxCalibrationMin[i] = RSSI_ADC_READING_MIN;
  }
  setSaveRequired();
}

void rssiCalibrationUpdate() {
  // Пройтись по всем каналам. Из пустых каналов взять Min. Из занятых Max.
  if (UNLIKELY(isCurrentlyCalibrating && calibrationTimer.hasTicked())) {
    for (uint8_t i = 0; i < getNumReceivers(); i++) {
      if (getRSSI(i) < EepromSettings.RxCalibrationMin[i])
        EepromSettings.RxCalibrationMin[i] = getRSSI(i);

      if (getRSSI(i) > EepromSettings.RxCalibrationMax[i]){
        EepromSettings.RxCalibrationMax[i] = getRSSI(i);
        maxFreq=channelFreqTable[calibrationFreqIndex];
      }
    }
    calibrationFreqIndex++;
    if (calibrationFreqIndex < 8*8) { // 8*8 = 8 bands * 8 channels = total number of freq in channelFreqTable.
      setModuleFrequencyAll(channelFreqTable[calibrationFreqIndex]);
      calibrationTimer.reset();

    } else {
      for (int i = 0; i < getNumReceivers(); i++) {
        setModuleChannelBand(i);
        // Prevent min > max
        EepromSettings.RxCalibrationMax[i] = MAX(EepromSettings.RxCalibrationMax[i], EepromSettings.RxCalibrationMin[i] + 1);
        Serial.print("Freq with max:");
        Serial.println(maxFreq);
        Serial.print("Min[");
        Serial.print( i );
        Serial.print("]: ");
        Serial.print(EepromSettings.RxCalibrationMin[i]);
        Serial.print(" Max: ");
        Serial.println(EepromSettings.RxCalibrationMax[i]);
      }
      isCurrentlyCalibrating = false;
      setSaveRequired();
      setDisplayScreenNumber(0);
      setRXADCfilter(EepromSettings.RXADCfilter);
    }
  }
}

int getcalibrationFreqIndex() {
  return calibrationFreqIndex;
}
