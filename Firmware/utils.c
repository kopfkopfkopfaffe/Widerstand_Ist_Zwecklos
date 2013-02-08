/*
 * utils.c
 *
 *  Created on: Jul 19, 2010
*      Author: KopfKopfKopfAffe
 */
#include "includes/utils.h"
#include <util/delay.h>

void long_delay(uint16_t ms) {
	for (; ms > 0; ms--)
		_delay_ms(1);
}


uint16_t make_u16(uint8_t high_byte, uint8_t low_byte) {
	return (((uint16_t) high_byte) << 8 | low_byte);
}

uint8_t compare_arrays(char first_array[], const char second_array[]){
	uint8_t same = 1;
	for (uint8_t i = 0; i < 3; i++) {
		if(!(first_array[i] == second_array[i])){
			same = 0;
		}
	}
	return same;
}
