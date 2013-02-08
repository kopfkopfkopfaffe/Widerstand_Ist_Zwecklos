/*
 * utils.h
 *
 *  Created on: Jul 19, 2010
*      Author: KopfKopfKopfAffe
 */

#ifndef UTILS_H_
#define UTILS_H_
#include <stdio.h>
#include <avr/io.h>
void long_delay(uint16_t ms);
uint16_t make_u16(uint8_t high_byte, uint8_t low_byte);
uint8_t compare_arrays(char first_array[], const char second_array[]);
#endif /* UTILS_H_ */
