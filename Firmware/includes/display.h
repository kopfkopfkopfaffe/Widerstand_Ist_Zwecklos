/*
 * display.h
 *
 *  Created on: Jul 31, 2010
*      Author: KopfKopfKopfAffe
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_
void updatevalue(float newvalue);
void initdisplay(void);
void refreshdisplay(void);
void setvalue(uint16_t thevalue);
void setdigits(uint8_t color0, uint8_t color1, uint8_t colormag);
void setmanual(uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3);
void blankdisplay(void);
#endif /* DISPLAY_H_ */
