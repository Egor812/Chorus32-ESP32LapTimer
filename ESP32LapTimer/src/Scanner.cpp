#include "Scanner.h"

#include <rom/ets_sys.h>
#include "ADC.h"
#include "RX5808.h"
#include "settings_eeprom.h"


static uint16_t RX_RSSI[64];
uint8_t bestFreqNum=0xFF;
uint16_t maxRSSI;


void rssiScan(void){
  maxRSSI=0;
  setRXADCfilter(LPF_10Hz);  
  for(uint8_t j=0; j<8; j++){
    for(uint8_t i=0; i<8; i++){
      setModuleChannelBand(i, j, 0);
      ets_delay_us(50000);
      RX_RSSI[j*8+i] = getRSSI(0);
      if( RX_RSSI[j*8+i] > maxRSSI ) {
        maxRSSI=RX_RSSI[j*8+i];
        bestFreqNum = j*8+i;
      }      
    }
  }
}

uint16_t getScannedRSSI(uint8_t i){
    return RX_RSSI[i];
}

uint8_t getBestScannedCh(void){
  return bestFreqNum;
}