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
#include "OLED.h"

#include <Wire.h>
#include "SSD1306.h"
#include "Font.h"
#include "Timer.h"
#include "Screensaver.h"
#include "ADC.h"
#include "settings_eeprom.h"
#include "RX5808.h"
#include "Calibration.h"
#include "TimerWebServer.h"
#include "Utils.h"
#include "Scanner.h"
#include "Comms.h"
#include "Matrix.h"

static uint8_t oledRefreshTime = 50;
static uint32_t last_input_ms = 0;
static bool display_standby_status = false;

static Timer oledTimer = Timer(oledRefreshTime);

static SSD1306 display(0x3c, 21, 22);  // 21 and 22 are default pins

typedef struct oled_page_s {
  void* data;
  void (*init)(void* data);
  void (*draw_page)(void* data);
  void (*process_input)(void* data, uint8_t index, uint8_t type);
} oled_page_t;

static struct rxPageData_s {
  uint8_t currentPilotNumber;
} rxPageData;



oled_page_t oled_pages[] = {
  {NULL, NULL, summary_page_update, next_page_input},
  {NULL, NULL, adc_page_update, next_page_input},
  {NULL, NULL, calib_page_update, calib_page_input},
  {NULL, NULL, airplane_page_update, airplane_page_input},
  {NULL, NULL, scan_page_update, scan_page_input},
  {&rxPageData, rx_page_init, rx_page_update, rx_page_input}
};

#define NUM_OLED_PAGES (sizeof(oled_pages)/sizeof(oled_pages[0]))

static uint8_t current_page = 0;

static void oledNextPage() {
  current_page = (current_page + 1) % NUM_OLED_PAGES;
}

void oledSetup(void) {
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.drawFastImage(0, 0, 128, 64, ChorusLaptimerLogo_Screensaver);
  display.display();
  display.setFont(Dialog_plain_9);
  
  for(uint8_t i = 0; i < NUM_OLED_PAGES; ++i) {
    if(oled_pages[i].init) {
      oled_pages[i].init(oled_pages[i].data);
    }
  }
  last_input_ms = millis();
}

void OLED_CheckIfUpdateReq() {
  if (oledTimer.hasTicked()) {
    if(millis() - last_input_ms > getDisplayTimeout() && getDisplayTimeout() != 0) {
      if(!display_standby_status) {
        display.displayOff(); // going in standby
        display_standby_status = true;
      }
    } else if(display_standby_status) {
        display.displayOn();
        display_standby_status = false;
    }
    if(!display_standby_status) {
      if(oled_pages[current_page].draw_page) {
        display.clear();
        oled_pages[current_page].draw_page(oled_pages[current_page].data);
        display.display();
      }
    }
    oledTimer.reset();
  }
}

void oledInjectInput(uint8_t index, uint8_t type) {
  if(!display_standby_status) {
    if(oled_pages[current_page].process_input) {
      oled_pages[current_page].process_input(oled_pages[current_page].data, index, type);
    }
  } else { // turn display on again
    display.displayOn();
  }
  last_input_ms = millis();
}

void next_page_input(void* data, uint8_t index, uint8_t type) {
  (void)data;
  if(index == 0 && type == BUTTON_SHORT) {
    oledNextPage();
  }
}

void rx_page_init(void* data) {
  rxPageData_s* my_data = (rxPageData_s*) data;
  my_data->currentPilotNumber = 0;
}

void rx_page_input(void* data, uint8_t index, uint8_t type) {
  rxPageData_s* my_data = (rxPageData_s*) data;
  if(index == 0 && type == BUTTON_SHORT) { //b1
    ++my_data->currentPilotNumber;
    if(my_data->currentPilotNumber >= getNumReceivers()) {
      oledNextPage();
      my_data->currentPilotNumber = 0;
    }
  }
  else if(index == 1 && type == BUTTON_SHORT) { //b2
    incrementRxFrequency(my_data->currentPilotNumber);
  }
  else if(index == 1 && type == BUTTON_LONG) { //b2
    incrementRxBand(my_data->currentPilotNumber);
  }
}

void rx_page_update(void* data) {
  // Gather Data
  rxPageData_s* my_data = (rxPageData_s*) data;
  uint8_t frequencyIndex = getRXChannel(my_data->currentPilotNumber) + (8 * getRXBand(my_data->currentPilotNumber));
  uint16_t frequency = channelFreqTable[frequencyIndex];

  // Display things
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Settings for RX" + String(my_data->currentPilotNumber + 1));
  display.drawString(0, 18, getBandLabel(getRXBand(my_data->currentPilotNumber)) + String(getRXChannel(my_data->currentPilotNumber) + 1) + " - " + frequency);
  if (getRSSI(my_data->currentPilotNumber) < 600) {
    display.drawProgressBar(48, 35, 120 - 42, 8, map(600, 600, 3500, 0, 85));
  } else {
    display.drawProgressBar(48, 35, 120 - 42, 8, map(getRSSI(my_data->currentPilotNumber), 600, 3500, 0, 85));
  }
  display.setFont(Dialog_plain_9);
  display.drawString(0,35, "RSSI: " + String(getRSSI(my_data->currentPilotNumber) / 12));
  display.drawVerticalLine(45 + map(getRSSIThreshold(my_data->currentPilotNumber), 600, 3500, 0, 85),  35, 8); // line to show the RSSIthresholds
  display.drawString(0,46, "Btn2 SHORT - Channel.");
  display.drawString(0,55, "Btn2 LONG  - Band.");
}

void scan_page_update(void* data){
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Dialog_plain_9);
  display.drawString(0,0, "RX0 RSSI:");
  display.drawString(0,55, "Press Btn2 to start");
  display.drawString(10,10, "R");
  display.drawString(20,10, "A");
  display.drawString(30,10, "B");
  display.drawString(40,10, "E");
  display.drawString(50,10, "F");
  display.drawString(60,10, "D");
  for(byte j=0; j<8; j++){
    for(byte i=0; i<8; i++){
      display.drawVerticalLine( 10+j*10+i, 20, map( getScannedRSSI(i+j*8), 600, 3500, 0, 70) );
    }
  }
  uint8_t bCh = getBestScannedCh();
  if( bCh!=0xFF) display.drawString(70,0, "max:" + getBandLabel(bCh/8) + String(bCh%8 + 1));
}

void summary_page_update(void* data) {
  // Display on time
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  // Hours
  if (millis() / 3600000 < 10) {
    display.drawString(0, 0, "0" + String(millis() / 3600000) + ":");
  } else {
    display.drawString(0, 0, String(millis() / 3600000) + ":");
  }
  // Mins
  if (millis() % 3600000 / 60000 < 10) {
    display.drawString(18, 0, "0" + String(millis() % 3600000 / 60000) + ":");
  } else {
    display.drawString(18, 0, String(millis() % 3600000 / 60000) + ":");
  }
  // Seconds
  if (millis() % 60000 / 1000 < 10) {
    display.drawString(36, 0, "0" + String(millis() % 60000 / 1000));
  } else {
    display.drawString(36, 0, String(millis() % 60000 / 1000));
  }
  
  #ifdef LED_MATRIX
    if(UNLIKELY(!isInRaceMode())) {
      drawMatrixDigit(0, millis() % 60000 / 1000, 1); 
    }
  #endif
  

  // Voltage
  if (getADCVBATmode() != 0) {
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(127, 0, String(getVbatFloat(), 2) + "V");
  }

  if (getADCVBATmode() == INA219) {
    display.drawString(90, 0, String(getMaFloat()/1000, 2) + "A");
  }

  if( isSoundEnabled() )     display.drawString(56, 0, "S");
  else      display.drawString(56, 0, "m");

  if( isShouldWaitForFirstLap() )     display.drawString(64, 0, "b"); //  Start | drone ... | Finish
  else      display.drawString(64, 0, "d"); // drone | Start ... | Finish

  
  // Rx modules
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  #define RSSI_BAR_LENGTH (127 - 42)
  #define RSSI_BAR_HEIGHT 8
  #define RSSI_BAR_X_OFFSET 40
  for (int i = 0; i < getNumReceivers(); i++) {
    display.drawString(0, 9 + i * 9, getBandLabel(getRXBand(i)) + String(getRXChannel(i) + 1) + ", " + String(getRSSI(i) / 12) );
    display.drawProgressBar(RSSI_BAR_X_OFFSET, 10 + i * 9, RSSI_BAR_LENGTH, RSSI_BAR_HEIGHT, map(getRSSI(i), RSSI_ADC_READING_MIN, RSSI_ADC_READING_MAX, 0, 100));
    display.drawVerticalLine(RSSI_BAR_X_OFFSET + map(MAX(getRSSIThreshold(i), RSSI_ADC_READING_MIN), RSSI_ADC_READING_MIN, RSSI_ADC_READING_MAX, 0, RSSI_BAR_LENGTH),  10 + i * 9, RSSI_BAR_HEIGHT); // line to show the RSSIthresholds
  }
}

void adc_page_update(void* data) {
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "ADC loop " + String(getADCLoopCount() * (1000.0 / oledRefreshTime)) + " Hz");
  setADCLoopCount(0);
  display.drawString(0, 9, String(getMaFloat()) + " mA");
}

void calib_page_update(void* data) {
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Frequency - " + String(channelFreqTable[getcalibrationFreqIndex()]) + "Hz");
  display.drawString(0,  9, "Min = " + String(EepromSettings.RxCalibrationMin[0]) + ", Max = " + String(EepromSettings.RxCalibrationMax[0]));
  display.drawString(0, 18, "Min = " + String(EepromSettings.RxCalibrationMin[1]) + ", Max = " + String(EepromSettings.RxCalibrationMax[1]));
  display.drawString(0, 27, "Min = " + String(EepromSettings.RxCalibrationMin[2]) + ", Max = " + String(EepromSettings.RxCalibrationMax[2]));
  display.drawString(0, 36, "Min = " + String(EepromSettings.RxCalibrationMin[3]) + ", Max = " + String(EepromSettings.RxCalibrationMax[3]));
  display.drawString(0, 45, "Min = " + String(EepromSettings.RxCalibrationMin[4]) + ", Max = " + String(EepromSettings.RxCalibrationMax[4]));
  display.drawString(0, 54, "Min = " + String(EepromSettings.RxCalibrationMin[5]) + ", Max = " + String(EepromSettings.RxCalibrationMax[5]));
}

void calib_page_input(void* data, uint8_t index, uint8_t type) {
  (void)data;
  if(index == 1 && type == BUTTON_SHORT) { //b2
    rssiCalibration();  
  }
  else if(index == 1 && type == BUTTON_LONG) { //b2
    rssiCalibrationReset();
  }  
  else {
    next_page_input(data, index, type);
  }
}

void scan_page_input(void* data, uint8_t index, uint8_t type) {
  (void)data;
  if(index == 1 && type == BUTTON_SHORT) { //b2
    rssiScan();  
  }
  else {
    next_page_input(data, index, type);
  }
}

void airplane_page_update(void* data) {
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Airplane Mode Settings:");
  display.drawString(0, 15, "Long Press Button 2 to");
  display.drawString(0, 26, "toggle Airplane mode.");
  if (!isAirplaneModeOn()) {
    display.drawString(0, 42, "Airplane Mode: OFF");
    display.drawString(0, 51, "WiFi: ON  | Draw: " + String(getMaFloat()/1000, 2) + "A");
  } else {
    display.drawString(0, 42, "Airplane Mode: ON");
    display.drawString(0, 51, "WiFi: OFF  | Draw: " + String(getMaFloat()/1000, 2) + "A");
  }
}

void airplane_page_input(void* data, uint8_t index, uint8_t type) {
  if(index == 1 && type == BUTTON_LONG) {
    toggleAirplaneMode();
  } else {
    next_page_input(data, index, type);
  }
}

void incrementRxFrequency(uint8_t currentRXNumber) {
  uint8_t currentRXChannel = getRXChannel(currentRXNumber);
  uint8_t currentRXBand = getRXBand(currentRXNumber);
  currentRXChannel++;
  if (currentRXChannel >= 8) {
    //currentRXBand++;
    currentRXChannel = 0;
  }
  if (currentRXBand >= 7 && currentRXChannel >= 2) {
    currentRXBand = 0;
    currentRXChannel = 0;
  }
  setModuleChannelBand(currentRXChannel,currentRXBand,currentRXNumber);
}
void incrementRxBand(uint8_t currentRXNumber) {
  uint8_t currentRXChannel = getRXChannel(currentRXNumber);
  uint8_t currentRXBand = getRXBand(currentRXNumber);
  currentRXBand++;
  if (currentRXBand >= 8) {
    currentRXBand = 0;
  }
  setModuleChannelBand(currentRXChannel,currentRXBand,currentRXNumber);
}

void setDisplayScreenNumber(uint16_t num) {
  if(num < NUM_OLED_PAGES) {
    current_page = num;
  }
}

uint16_t getDisplayScreenNumber() {
  return current_page;
}

