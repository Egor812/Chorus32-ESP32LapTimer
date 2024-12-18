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
#ifndef __BEEPER_H__
#define __BEEPER_H__

#include <stdint.h>


void beep();
/*void doubleBeep();
void chirps();
void fiveBeep();*/
void beepBits(uint32_t);
void beep1x500();
void beep2x500();
void beep1x1000();
void beep2x250();
void beep3x250();
void beep1x250();
void beepYes();
void beepNo();
void beeperUpdate();

#endif // __BEEPER_H__
