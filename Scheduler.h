/*
    AVR-Scheduler created in ProLab
    Copyright (C) 2016 Jonathan Schostak, Sven Rotter, Andreas Liu, Patrick Artz, Karsten Eckhoff, Talha Kösker, Geronimo Bergk, Stephan Bauroth
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    jonathan@schostak.org
*/

#ifndef _TIMING_H_
#define _TIMING_H_

#include <stdint.h>

/* must be called before any other action */
void init_timer 		( void );

/* blocking delay-function */
void delay			( uint16_t ms );

/* schedule a function in the given time. Remember the argument must somehow be available in memory on execution! */
uint8_t schedule_function	( void (*functionPtr)(void*), void *args, uint16_t ms );

#endif /*_TIMING_H_ */
