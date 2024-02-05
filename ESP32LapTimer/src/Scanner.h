#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <stdint.h>

uint16_t getScannedRSSI(uint8_t i);
uint8_t getBestScannedCh(void);
void rssiScan(void);

#endif // __SCANNER_H__