/*
 * widerstand.h
 *
 *  Created on: Dec 20, 2010
 *      Author: KopfKopfKopfAffe
 */

#ifndef WIDERSTAND_H_
#define WIDERSTAND_H_
uint16_t getclosestvalue(uint16_t searchvalue);
uint16_t readADC(uint8_t channel);
void writeeeprom(void);
void setledtocolor(uint8_t ledid, uint8_t colorid);
#endif /* WIDERSTAND_H_ */
