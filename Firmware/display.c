/*
 * display.c
 *
 *  Created on: Jul 31, 2010
 *  Author: KopfKopfKopfAffe
 */
#include <avr/io.h>
#include <stdlib.h>
#include "includes/display.h"
#include "includes/utils.h"

uint8_t displaycontent[4];
uint8_t currentdigit = 3;
uint8_t holdtime = 0;
uint8_t decimalpoint = 0;
uint8_t tempcontent = 0;

//digit data lines:
//IA: PD0
//IB: PD1
//IC: PD2
//ID: PD3
//DP: PB5
//
//display select:
//D1: PC3
//D2: PC4
//D3: PD4
//D4: PB4

void initdisplay() {
	//debug: fill array
	displaycontent[0] = 0;
	displaycontent[1] = 1;
	displaycontent[2] = 2;
	displaycontent[3] = 3;
	//configure data directions
	DDRB |= (1 << PB4) | (1 << PB5);
	DDRC |= (1 << PC3) | (1 << PC4);
	DDRD |= (1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4);
}

void refreshdisplay() {

	holdtime++;
	if (holdtime > 100) {
		//no display active, all segments off
		PORTD &= ~((1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3) | (1
				<< PD4));
		PORTC &= ~((1 << PC3) | (1 << PC4));
		PORTB &= ~((1 << PB4) | (1 << PB5));
		//set data pins
		PORTD |= (displaycontent[currentdigit]);
		//set display select pin iff number is smaller than 10. numbers above 10 are blanking the depending display
		if (displaycontent[currentdigit] < 10) {
			//turn on current display
			if (currentdigit == 0)
				PORTC |= (1 << PC3);
			if (currentdigit == 1)
				PORTC |= (1 << PC4);
			if (currentdigit == 2)
				PORTD |= (1 << PD4);
			if (currentdigit == 3)
				PORTB |= (1 << PB4);

			if (decimalpoint && (currentdigit == 2))
				PORTB |= (1 << PB5);
		}
		currentdigit++;
		holdtime = 0;
	}
	if (currentdigit > 3)
		currentdigit = 0;
}

void updatevalue(float newvalue) {
	//if newvalue is < 100, the decimal place is needed. shift the value to the right so that it is an integer, turn on decimal point
	/*
	 if (newvalue < 100) {
	 newvalue = newvalue * 10.0;
	 decimalpoint = 1;
	 } else {
	 decimalpoint = 0;
	 //turn off decimal point
	 }
	 */
	//calculate single decimal places
	setvalue((uint16_t) newvalue);
}

void blankdisplay(){
	displaycontent[0] = 10;
	displaycontent[1] = 10;
	displaycontent[2] = 10;
	displaycontent[3] = 10;
}

void setmanual(uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3){
	displaycontent[0] = d0;
	displaycontent[1] = d1;
	displaycontent[2] = d2;
	displaycontent[3] = d3;
}

void setdigits(uint8_t color0, uint8_t color1, uint8_t colormag) {
	//Digits: LEFT [D0][D1][D2][D3] RIGHT

	//no successfull measurement means all values for setdigits are 9. Therefor check that:
	if ((colormag==9)){
	blankdisplay();
	}

	//if resistor value is zero
	if (color0 == 0 && color1 == 0 ) {
			displaycontent[0] = 10;
			displaycontent[1] = 10;
			displaycontent[2] = 10;
			displaycontent[3] = color1;
	}
	//if only two digits, turn other ones off and use the right ones
	if (colormag == 0 && (color0 + color1) > 0 ) {
		displaycontent[0] = 10;
		displaycontent[1] = 10;
		displaycontent[2] = color0;
		displaycontent[3] = color1;
	}
	//if one zero, turn left display off, use middle ones, set right one to zero
	if (colormag == 1) {
		displaycontent[0] = 10;
		displaycontent[1] = color0;
		displaycontent[2] = color1;
		displaycontent[3] = 0;
	}
	//if two zeros, use all displays, both right ones zero
	if (colormag == 2) {
		displaycontent[0] = color0;
		displaycontent[1] = color1;
		displaycontent[2] = 0;
		displaycontent[3] = 0;
	}
	if (colormag > 2 && colormag < 9) {
		displaycontent[0] = color0;
		displaycontent[1] = color1;
		displaycontent[2] = 10;
		displaycontent[3] = colormag;
	}
}

void setvalue(uint16_t thevalue) {
	//changed to new pinout.
	displaycontent[3] = (thevalue % 10); // Modulo rechnen, dann den ASCII-Code von '0' addieren
	thevalue /= 10;
	displaycontent[2] = (thevalue % 10); // Modulo rechnen, dann den ASCII-Code von '0' addieren
	thevalue /= 10;
	displaycontent[0] = (thevalue % 10); // Modulo rechnen, dann den ASCII-Code von '0' addieren
	thevalue /= 10;
	displaycontent[1] = (thevalue % 10); // Modulo rechnen, dann den ASCII-Code von '0' addieren
	//thevalue /= 10;
	/*	for (uint8_t i = 0; i > 2 ; i++) {
	 displaycontent[i] = (thevalue % 10); // Modulo rechnen, dann den ASCII-Code von '0' addieren
	 thevalue /= 10;
	 }
	 */
}
