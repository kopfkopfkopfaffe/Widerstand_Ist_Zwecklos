/*
 * widerstand.c
 *
 *  Created on: Dec 20, 2010
*      Author: KopfKopfKopfAffe
 *
 *
 *
 *      SOMEHOW WORKS, IT HAS TIMEOUTS AND THE OUTPUT ONLY WORKS IN THE INTERRUPT. FUNNY...
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "includes/utils.h"
#include "includes/widerstand.h"
#include "includes/uart.h"
#include "includes/display.h"
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/pgmspace.h>
#ifndef F_CPU
#define F_CPU = 20000000UL
#endif
#define UART_BAUD_RATE 9600

// Variables for software-pwm
uint8_t reds[3] = { 0, 0, 0 }; //{ LED1, LED2, LED3} 0-32
uint8_t greens[3] = { 0, 0, 0 }; //{ LED1, LED2, LED3} 0-32
uint8_t blues[3] = { 0, 0, 0 }; //{ LED1, LED2, LED3} 0-32
uint8_t pwmcounter = 0;
uint8_t ledcounter = 0;

// Variables for openline animation
uint8_t openline = 0;
uint8_t animation0 = 0;
uint8_t animation1 = 15;
uint8_t animation2 = 30;

// Variables for resistor measurement
uint16_t milliseconds = 1;
char wert[8];
uint8_t status = 0; //0=keine Messung, 1=Messung erfolgreich, 2=Out of Bounds

// Values that are measured at different resistors
static const uint16_t pgmUValues[] PROGMEM = { 0, 56, 63, 80, 93, 114, 137,
		163, 187, 218, 249, 289, 337, 374, 417, 473, 520, 573, 622, 670, 708,
		747, 782, 815, 844, 870, 894, 917, 933, 948, 1091, 1109, 1127, 1150,
		1147, 1204, 1237, 1275, 1313, 1364, 1408, 1458, 1510, 1561, 1605, 1648,
		1690, 1731, 1770, 1804, 1834, 1867, 1889, 1910, 1929, 1944, 1957, 1967,
		1976, 1982, 1990, 1996, 2020 };
// Ring colors (ring1, ring2, ring3 aka value, value, #zeroes)
static const uint16_t pgmRingColors[] PROGMEM = { 000, 100, 120, 150, 180, 220,
		270, 330, 390, 470, 560, 680, 820, 101, 121, 151, 181, 221, 271, 331,
		391, 471, 561, 681, 821, 102, 122, 152, 182, 222, 272, 332, 392, 472,
		562, 682, 822, 103, 123, 153, 183, 223, 273, 333, 393, 473, 563, 683,
		823, 104, 124, 154, 184, 224, 274, 334, 394, 474, 564, 684, 824, 105,
		999 };

// Gradient maps for idle animation:
// This tables can be generated with the python script in ../Scripts
static const uint8_t pgmRedGradient[] PROGMEM = { 29, 29, 29, 30, 28, 29, 29,
		29, 30, 29, 29, 29, 28, 29, 30, 29, 30, 30, 31, 30, 31, 30, 27, 26, 25,
		24, 22, 20, 20, 18, 17, 16, 14, 12, 11, 10, 8, 8, 6, 5, 4, 2, 1, 1, 0,
		0, 0, 0, 1, 1, 2, 2, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 1, 5, 5, 6, 7, 9,
		10, 11, 13, 15, 16, 19, 19, 20, 21, 23, 23, 25, 26, 29, 30, 31, 31, 31,
		30, 30, 30, 29, 29, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, };
static const uint8_t
		pgmGreenGradient[] PROGMEM = { 4, 4, 6, 7, 8, 9, 11, 11, 14, 15, 16,
				17, 20, 21, 22, 23, 24, 26, 27, 29, 30, 30, 30, 31, 30, 31, 31,
				31, 31, 31, 30, 31, 31, 31, 32, 31, 32, 32, 31, 32, 31, 31, 32,
				32, 32, 32, 32, 32, 31, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31,
				31, 31, 31, 32, 31, 29, 29, 28, 27, 25, 24, 22, 22, 19, 19, 17,
				16, 14, 13, 11, 11, 9, 8, 6, 5, 3, 3, 1, 0, 0, 0, 0, 0, 0, 1,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0,
				0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, };
static const uint8_t
		pgmBlueGradient[] PROGMEM = { 0, 0, 0, 1, 1, 2, 1, 1, 1, 2, 0, 1, 1, 2,
				3, 1, 0, 0, 0, 0, 2, 2, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1,
				0, 0, 0, 1, 1, 0, 0, 1, 1, 3, 3, 3, 5, 7, 8, 11, 11, 13, 13,
				16, 16, 21, 21, 23, 23, 25, 25, 27, 29, 30, 32, 32, 31, 32, 31,
				30, 30, 33, 33, 32, 31, 31, 30, 31, 31, 32, 31, 32, 31, 30, 30,
				32, 31, 32, 32, 30, 31, 31, 32, 30, 30, 31, 31, 30, 30, 30, 31,
				29, 29, 30, 31, 30, 31, 29, 29, 29, 26, 24, 23, 22, 20, 20, 18,
				16, 15, 15, 11, 11, 9, 6, 6, 6, 6, };

ISR (TIMER0_COMPA_vect)
{
	// Handle display
	refreshdisplay();
	// Compare all of em pwm-values to counter
	if (pwmcounter > reds[ledcounter]) {
		PORTB &= ~(1 << PB0); // Set R-Chan to 0
	}
	if (pwmcounter > greens[ledcounter]) {
		PORTB &= ~(1 << PB2); // Set G-Chan to 0
	}
	if (pwmcounter > blues[ledcounter]) {
		PORTB &= ~(1 << PB1); // Set B-Chan to 0
	}
	// Increment pwm-counter. This is used to compare the pwm-values for each channel and toggle the ports on match.
	pwmcounter++;
	// If pwmcounter = 0, reset all chans to 1
	if (pwmcounter > 31) {
		ledcounter++;
		if (ledcounter > 2) {
			ledcounter = 0;
		}
		PORTC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2));
		pwmcounter = 0;
		if (reds[ledcounter]) {
			PORTB |= (1 << PB0);
		}
		if (greens[ledcounter]) {
			PORTB |= (1 << PB2);
		}
		if (blues[ledcounter]) {
			PORTB |= (1 << PB1);
		}
		PORTC |= (1 << ledcounter);
	}
}

ISR (TIMER1_COMPA_vect)
{
	if (openline) {
		animation0 ++;
		// If (animation0 > 129) animation0 = 0;
		animation1 ++;
		// If (animation1 > 129) animation1 = 0;
		animation2 ++;
		// If (animation2 > 129) animation2 = 0;
		reds[0] =  pgm_read_byte(&pgmRedGradient[animation0/2]);
		greens[0] =  pgm_read_byte(&pgmGreenGradient[animation0/2]);
		blues[0] =  pgm_read_byte(&pgmBlueGradient[animation0/2]);

		reds[1] =  pgm_read_byte(&pgmRedGradient[animation1/2]);
		greens[1] =  pgm_read_byte(&pgmGreenGradient[animation1/2]);
		blues[1] =  pgm_read_byte(&pgmBlueGradient[animation1/2]);

		reds[2] =  pgm_read_byte(&pgmRedGradient[animation2/2]);
		greens[2] =  pgm_read_byte(&pgmGreenGradient[animation2/2]);
		blues[2] =  pgm_read_byte(&pgmBlueGradient[animation2/2]);
	}
}

int main(void) {

	// Eeprom storage write operations (only comment in once if you want to update the eeprom content)
	// Eeprom contend can be generated using voltage_divider_calculations.ods in ../
	//writeeeprom();

	// Configure Timer 0
	TCCR0B |= (1 << CS00); // Prescaler 0
	// Enable Compare Interrupt
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	// set Flag on Compare Interrupt
	TIFR0 |= (1 << OCF0A);
	// Set Timer Compare Value
	OCR0A = 255;

	// Configure Timer 1
	// One interrupt per millisecond@20mhz
	TCCR1B = (1 << CS10) | (1 << WGM12); // Prescaler 1024
	OCR1AH = 0xFF;
	OCR1AL = 0xFF;
	TIMSK1 = (1 << OCIE1A);

	// Configure outputs
	DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
	DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2);

	uint16_t adcresult;

	initdisplay();

	//uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));

	// Enable interrupts
	sei();

	//Debug output
	/*
	 //uart_puts("Widerstawerwerwerewrewreklos!\r\n");
	 uart_puts("Reading Progmem: \r\n");
	 uint16_t UValue;
	 uint16_t RColor;
	 for (i = 0; i < 60; i++) {
	 UValue = pgm_read_word (&pgmUValues[i]);
	 RColor = pgm_read_word (&pgmRingColors[i]);
	 uart_puts(itoa(UValue, wert, 10));
	 uart_puts("  ");
	 uart_puts(itoa(RColor/100, wert, 10));
	 uart_puts("  ");
	 uart_puts(itoa((RColor%100)/10, wert, 10));
	 uart_puts("  ");
	 uart_puts(itoa(RColor%10, wert, 10));
	 uart_puts("\r\n");
	 }

	 uart_puts("Widerstand ist zwecklos!\r\n");

	 for (i = 0; i < 0x3C; i++) {
	 j = make_u16(eeprom_read_byte(i + 0x3C), eeprom_read_byte(i));
	 uart_puts(itoa(j, wert, 10));
	 uart_puts("  ");
	 uart_puts(itoa(eeprom_read_byte(i + 0x3c + 0x3c), wert, 10));
	 uart_puts("  ");
	 uart_puts(itoa(eeprom_read_byte(i + 0x3c + 0x3c + 0x3c), wert, 10));
	 uart_puts("  ");
	 uart_puts(itoa(eeprom_read_byte(i + 0x3c + 0x3c + 0x3c + 0x3c), wert,
	 10));
	 uart_puts("\r\n");
	 }

	 for (i = 0; i < 10; i++) {
	 setledtocolor(0, i);
	 setledtocolor(1, i);
	 setledtocolor(2, i);
	 setmanual(i,i,i,i);
	 long_delay(1000);
	 }
	 */

	uint16_t myaddress = 0;
	while (1) {
		long_delay(100);
		// Measure with low resistor
		DDRD |= (1 << PD6);
		PORTD |= (1 << PD6);
		long_delay(10);
		adcresult = readADC(5);
		myaddress = getclosestvalue(adcresult);
		if (adcresult > 948) {
			// Low resistor off
			PORTD &= ~(1 << PD6);
			DDRD &= ~(1 << PD6);
			// Measure with high resistor
			DDRD |= (1 << PD7);
			PORTD |= (1 << PD7);
			long_delay(50);
			// Read new value
			adcresult = readADC(5);
			myaddress = getclosestvalue(adcresult + 1000);

			//Debug:
			/*
			 uart_puts("High Resistor: ");
			 uart_puts(itoa(adcresult + 1000, wert, 10));
			 uart_puts(", Result: ");
			 uart_puts(itoa(myaddress, wert, 10));
			 uart_puts("\r\n");
			 */
		} else {
			//low resistor is ok

			//Debug:
			/*
			 uart_puts("Low Resistor: ");
			 uart_puts(itoa(adcresult, wert, 10));
			 uart_puts(", Result: ");
			 uart_puts(itoa(myaddress, wert, 10));
			 uart_puts("\r\n");
			 */
		}
		// High resistor off
		PORTD &= ~(1 << PD7);
		DDRD &= ~(1 << PD7);
		// Low resistor off
		PORTD &= ~(1 << PD6);
		DDRD &= ~(1 << PD6);

		// Set colors:
		uint16_t RColor;
		RColor = pgm_read_word (&pgmRingColors[myaddress]);


		// Check if open line (aka all values 9 aka first or last value 9)
		if (RColor == 999) {
			blankdisplay();
			// Switch to openline mode of not already there
			if (!openline)
				openline = 1;
		} else {
			// If not open line, leave openline mode of not already left
			if (openline)
				openline = 0;
			setledtocolor(0, RColor / 100);
			setledtocolor(1, (RColor % 100) / 10);
			setledtocolor(2, RColor % 10);
		}

		setdigits(RColor / 100, (RColor % 100) / 10, RColor % 10);

	}

}

// This function takes a color code and an led address and sets the led to the desired color
void setledtocolor(uint8_t ledid, uint8_t colorid) {
	//black
	if (colorid == 0) {
		reds[ledid] = 0;
		greens[ledid] = 0;
		blues[ledid] = 0;
		// brown
	} else if (colorid == 1) {
		reds[ledid] = 15;
		greens[ledid] = 8;
		blues[ledid] = 1;
		// red
	} else if (colorid == 2) {
		reds[ledid] = 30;
		greens[ledid] = 0;
		blues[ledid] = 0;
		// orange
	} else if (colorid == 3) {
		reds[ledid] = 30;
		greens[ledid] = 13;
		blues[ledid] = 0;
		// yellow
	} else if (colorid == 4) {
		reds[ledid] = 25;
		greens[ledid] = 30;
		blues[ledid] = 0;
		// green
	} else if (colorid == 5) {
		reds[ledid] = 0;
		greens[ledid] = 30;
		blues[ledid] = 0;
		// blue
	} else if (colorid == 6) {
		reds[ledid] = 0;
		greens[ledid] = 0;
		blues[ledid] = 30;
		// violet
	} else if (colorid == 7) {
		reds[ledid] = 15;
		greens[ledid] = 0;
		blues[ledid] = 30;
		// grey
	} else if (colorid == 8) {
		reds[ledid] = 5;
		greens[ledid] = 5;
		blues[ledid] = 5;
		// white
	} else if (colorid == 9) {
		reds[ledid] = 30;
		greens[ledid] = 30;
		blues[ledid] = 30;
	} else {
		reds[ledid] = 0;
		greens[ledid] = 0;
		blues[ledid] = 0;
	}
}

// This function searches for the closest lookup value to the searchvalue
uint16_t getclosestvalue(uint16_t searchvalue) {
	uint16_t address = 0;
	uint16_t storedvalue;
	uint16_t difference = 1000;
	uint16_t tempdifference = 0;
	uint16_t i;
	// Iterate over all page slots
	for (i = 0; i < 63; i++) {
		// Get values from eeprom and assemble them to 16 bit number
		// OLD: from eeprom
		// storedvalue = make_u16(eeprom_read_byte(i + 0x3C), eeprom_read_byte(i));
		// NEW:
		storedvalue = pgm_read_word (&pgmUValues[i]);
		// Calculate difference
		tempdifference = abs(storedvalue-searchvalue);
		if (difference > tempdifference) {
			// If difference is smaller than previous difference, store difference
			// the current address is the best found so far.
			difference = tempdifference;
			address = i;
		}

	}
	return address;
}

uint16_t readADC(uint8_t channel) {
	uint8_t i;
	uint16_t result = 0;

	// Activate ADC, divider to 64
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

	// Select ADC channel
	// Use internal voltage reference (2,56 V)
	ADMUX = channel | (1 << REFS1) | (1 << REFS0);

	// Init ADC, perform dummy readout
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC))
		;

	// 3 measurements per channel
	// Calculate average
	for (i = 0; i < 3; i++) {
		// Measure
		ADCSRA |= (1 << ADSC);
		// Wait for result...
		while (ADCSRA & (1 << ADSC))
			;
		result += ADCW;
	}

	// Deactivate ADC
	ADCSRA &= ~(1 << ADEN);

	result /= 3;

	return result;
}

void writeeeprom() {
	/*	eeprom_write_byte(0x0, 0x40);
	 eeprom_write_byte(0x1, 0x4B);
	 eeprom_write_byte(0x2, 0x5D);
	 eeprom_write_byte(0x3, 0x6D);
	 eeprom_write_byte(0x4, 0x82);
	 eeprom_write_byte(0x5, 0x9C);
	 eeprom_write_byte(0x6, 0xB8);
	 eeprom_write_byte(0x7, 0xD3);
	 eeprom_write_byte(0x8, 0xF4);
	 eeprom_write_byte(0x9, 0x16);
	 eeprom_write_byte(0xA, 0x69);
	 eeprom_write_byte(0xB, 0x99);
	 eeprom_write_byte(0xC, 0xC7);
	 eeprom_write_byte(0xD, 0x00);
	 eeprom_write_byte(0xE, 0x2E);
	 eeprom_write_byte(0xF, 0x60);
	 eeprom_write_byte(0x10, 0x92);
	 eeprom_write_byte(0x11, 0xC0);
	 eeprom_write_byte(0x12, 0xE3);
	 eeprom_write_byte(0x13, 0x08);
	 eeprom_write_byte(0x14, 0x27);
	 eeprom_write_byte(0x15, 0x61);
	 eeprom_write_byte(0x16, 0x7A);
	 eeprom_write_byte(0x17, 0x8E);
	 eeprom_write_byte(0x18, 0xA2);
	 eeprom_write_byte(0x19, 0xB1);
	 eeprom_write_byte(0x1A, 0x35);
	 eeprom_write_byte(0x1B, 0x45);
	 eeprom_write_byte(0x1C, 0x57);
	 eeprom_write_byte(0x1D, 0x69);
	 eeprom_write_byte(0x1E, 0x7F);
	 eeprom_write_byte(0x1F, 0x97);
	 eeprom_write_byte(0x20, 0xD6);
	 eeprom_write_byte(0x21, 0xFC);
	 eeprom_write_byte(0x22, 0x23);
	 eeprom_write_byte(0x23, 0x55);
	 eeprom_write_byte(0x24, 0x81);
	 eeprom_write_byte(0x25, 0xB3);
	 eeprom_write_byte(0x26, 0xE8);
	 eeprom_write_byte(0x27, 0x1B);
	 eeprom_write_byte(0x28, 0x45);
	 eeprom_write_byte(0x29, 0x72);
	 eeprom_write_byte(0x2A, 0x9A);
	 eeprom_write_byte(0x2B, 0xEA);
	 eeprom_write_byte(0x2C, 0x0E);
	 eeprom_write_byte(0x2D, 0x2B);
	 eeprom_write_byte(0x2E, 0x4B);
	 eeprom_write_byte(0x2F, 0x62);
	 eeprom_write_byte(0x30, 0x78);
	 eeprom_write_byte(0x31, 0x8A);
	 eeprom_write_byte(0x32, 0x9A);
	 eeprom_write_byte(0x33, 0xA5);
	 eeprom_write_byte(0x34, 0xB0);
	 eeprom_write_byte(0x35, 0xB8);
	 eeprom_write_byte(0x36, 0xC7);
	 eeprom_write_byte(0x37, 0xCD);
	 eeprom_write_byte(0x38, 0xE5);
	 eeprom_write_byte(0x39, 0xFF);
	 eeprom_write_byte(0x3A, 0xFF);
	 eeprom_write_byte(0x3B, 0xFF);
	 eeprom_write_byte(0x3C, 0x0);
	 eeprom_write_byte(0x3D, 0x0);
	 eeprom_write_byte(0x3E, 0x0);
	 eeprom_write_byte(0x3F, 0x0);
	 eeprom_write_byte(0x40, 0x0);
	 eeprom_write_byte(0x41, 0x0);
	 eeprom_write_byte(0x42, 0x0);
	 eeprom_write_byte(0x43, 0x0);
	 eeprom_write_byte(0x44, 0x0);
	 eeprom_write_byte(0x45, 0x1);
	 eeprom_write_byte(0x46, 0x1);
	 eeprom_write_byte(0x47, 0x1);
	 eeprom_write_byte(0x48, 0x1);
	 eeprom_write_byte(0x49, 0x2);
	 eeprom_write_byte(0x4A, 0x2);
	 eeprom_write_byte(0x4B, 0x2);
	 eeprom_write_byte(0x4C, 0x2);
	 eeprom_write_byte(0x4D, 0x2);
	 eeprom_write_byte(0x4E, 0x2);
	 eeprom_write_byte(0x4F, 0x3);
	 eeprom_write_byte(0x50, 0x3);
	 eeprom_write_byte(0x51, 0x3);
	 eeprom_write_byte(0x52, 0x3);
	 eeprom_write_byte(0x53, 0x3);
	 eeprom_write_byte(0x54, 0x3);
	 eeprom_write_byte(0x55, 0x3);
	 eeprom_write_byte(0x56, 0x4);
	 eeprom_write_byte(0x57, 0x4);
	 eeprom_write_byte(0x58, 0x4);
	 eeprom_write_byte(0x59, 0x4);
	 eeprom_write_byte(0x5A, 0x4);
	 eeprom_write_byte(0x5B, 0x4);
	 eeprom_write_byte(0x5C, 0x4);
	 eeprom_write_byte(0x5D, 0x4);
	 eeprom_write_byte(0x5E, 0x5);
	 eeprom_write_byte(0x5F, 0x5);
	 eeprom_write_byte(0x60, 0x5);
	 eeprom_write_byte(0x61, 0x5);
	 eeprom_write_byte(0x62, 0x5);
	 eeprom_write_byte(0x63, 0x6);
	 eeprom_write_byte(0x64, 0x6);
	 eeprom_write_byte(0x65, 0x6);
	 eeprom_write_byte(0x66, 0x6);
	 eeprom_write_byte(0x67, 0x6);
	 eeprom_write_byte(0x68, 0x7);
	 eeprom_write_byte(0x69, 0x7);
	 eeprom_write_byte(0x6A, 0x7);
	 eeprom_write_byte(0x6B, 0x7);
	 eeprom_write_byte(0x6C, 0x7);
	 eeprom_write_byte(0x6D, 0x7);
	 eeprom_write_byte(0x6E, 0x7);
	 eeprom_write_byte(0x6F, 0x7);
	 eeprom_write_byte(0x70, 0x7);
	 eeprom_write_byte(0x71, 0x7);
	 eeprom_write_byte(0x72, 0x7);
	 eeprom_write_byte(0x73, 0x7);
	 eeprom_write_byte(0x74, 0x7);
	 eeprom_write_byte(0x75, 0xFF);
	 eeprom_write_byte(0x76, 0xFF);
	 eeprom_write_byte(0x77, 0xFF);
	 eeprom_write_byte(0x78, 0x1);
	 eeprom_write_byte(0x79, 0x1);
	 eeprom_write_byte(0x7A, 0x1);
	 eeprom_write_byte(0x7B, 0x1);
	 eeprom_write_byte(0x7C, 0x2);
	 eeprom_write_byte(0x7D, 0x2);
	 eeprom_write_byte(0x7E, 0x3);
	 eeprom_write_byte(0x7F, 0x3);
	 eeprom_write_byte(0x80, 0x4);
	 eeprom_write_byte(0x81, 0x5);
	 eeprom_write_byte(0x82, 0x8);
	 eeprom_write_byte(0x83, 0x1);
	 eeprom_write_byte(0x84, 0x1);
	 eeprom_write_byte(0x85, 0x1);
	 eeprom_write_byte(0x86, 0x1);
	 eeprom_write_byte(0x87, 0x2);
	 eeprom_write_byte(0x88, 0x2);
	 eeprom_write_byte(0x89, 0x3);
	 eeprom_write_byte(0x8A, 0x3);
	 eeprom_write_byte(0x8B, 0x4);
	 eeprom_write_byte(0x8C, 0x5);
	 eeprom_write_byte(0x8D, 0x8);
	 eeprom_write_byte(0x8E, 0x1);
	 eeprom_write_byte(0x8F, 0x1);
	 eeprom_write_byte(0x90, 0x1);
	 eeprom_write_byte(0x91, 0x1);
	 eeprom_write_byte(0x92, 0x2);
	 eeprom_write_byte(0x93, 0x2);
	 eeprom_write_byte(0x94, 0x3);
	 eeprom_write_byte(0x95, 0x3);
	 eeprom_write_byte(0x96, 0x4);
	 eeprom_write_byte(0x97, 0x5);
	 eeprom_write_byte(0x98, 0x8);
	 eeprom_write_byte(0x99, 0x1);
	 eeprom_write_byte(0x9A, 0x1);
	 eeprom_write_byte(0x9B, 0x1);
	 eeprom_write_byte(0x9C, 0x1);
	 eeprom_write_byte(0x9D, 0x2);
	 eeprom_write_byte(0x9E, 0x2);
	 eeprom_write_byte(0x9F, 0x3);
	 eeprom_write_byte(0xA0, 0x3);
	 eeprom_write_byte(0xA1, 0x4);
	 eeprom_write_byte(0xA2, 0x5);
	 eeprom_write_byte(0xA3, 0x8);
	 eeprom_write_byte(0xA4, 0x1);
	 eeprom_write_byte(0xA5, 0x1);
	 eeprom_write_byte(0xA6, 0x1);
	 eeprom_write_byte(0xA7, 0x1);
	 eeprom_write_byte(0xA8, 0x2);
	 eeprom_write_byte(0xA9, 0x2);
	 eeprom_write_byte(0xAA, 0x3);
	 eeprom_write_byte(0xAB, 0x3);
	 eeprom_write_byte(0xAC, 0x4);
	 eeprom_write_byte(0xAD, 0x5);
	 eeprom_write_byte(0xAE, 0x8);
	 eeprom_write_byte(0xAF, 0x1);
	 eeprom_write_byte(0xB0, 0xFF);
	 eeprom_write_byte(0xB1, 0xFF);
	 eeprom_write_byte(0xB2, 0xFF);
	 eeprom_write_byte(0xB3, 0xFF);
	 eeprom_write_byte(0xB4, 0x0);
	 eeprom_write_byte(0xB5, 0x2);
	 eeprom_write_byte(0xB6, 0x5);
	 eeprom_write_byte(0xB7, 0x8);
	 eeprom_write_byte(0xB8, 0x2);
	 eeprom_write_byte(0xB9, 0x7);
	 eeprom_write_byte(0xBA, 0x3);
	 eeprom_write_byte(0xBB, 0x9);
	 eeprom_write_byte(0xBC, 0x7);
	 eeprom_write_byte(0xBD, 0x6);
	 eeprom_write_byte(0xBE, 0x2);
	 eeprom_write_byte(0xBF, 0x0);
	 eeprom_write_byte(0xC0, 0x2);
	 eeprom_write_byte(0xC1, 0x5);
	 eeprom_write_byte(0xC2, 0x8);
	 eeprom_write_byte(0xC3, 0x2);
	 eeprom_write_byte(0xC4, 0x7);
	 eeprom_write_byte(0xC5, 0x3);
	 eeprom_write_byte(0xC6, 0x9);
	 eeprom_write_byte(0xC7, 0x7);
	 eeprom_write_byte(0xC8, 0x6);
	 eeprom_write_byte(0xC9, 0x2);
	 eeprom_write_byte(0xCA, 0x0);
	 eeprom_write_byte(0xCB, 0x2);
	 eeprom_write_byte(0xCC, 0x5);
	 eeprom_write_byte(0xCD, 0x8);
	 eeprom_write_byte(0xCE, 0x2);
	 eeprom_write_byte(0xCF, 0x7);
	 eeprom_write_byte(0xD0, 0x3);
	 eeprom_write_byte(0xD1, 0x9);
	 eeprom_write_byte(0xD2, 0x7);
	 eeprom_write_byte(0xD3, 0x6);
	 eeprom_write_byte(0xD4, 0x2);
	 eeprom_write_byte(0xD5, 0x0);
	 eeprom_write_byte(0xD6, 0x2);
	 eeprom_write_byte(0xD7, 0x5);
	 eeprom_write_byte(0xD8, 0x8);
	 eeprom_write_byte(0xD9, 0x2);
	 eeprom_write_byte(0xDA, 0x7);
	 eeprom_write_byte(0xDB, 0x3);
	 eeprom_write_byte(0xDC, 0x9);
	 eeprom_write_byte(0xDD, 0x7);
	 eeprom_write_byte(0xDE, 0x6);
	 eeprom_write_byte(0xDF, 0x2);
	 eeprom_write_byte(0xE0, 0x0);
	 eeprom_write_byte(0xE1, 0x2);
	 eeprom_write_byte(0xE2, 0x5);
	 eeprom_write_byte(0xE3, 0x8);
	 eeprom_write_byte(0xE4, 0x2);
	 eeprom_write_byte(0xE5, 0x7);
	 eeprom_write_byte(0xE6, 0x3);
	 eeprom_write_byte(0xE7, 0x9);
	 eeprom_write_byte(0xE8, 0x7);
	 eeprom_write_byte(0xE9, 0x6);
	 eeprom_write_byte(0xEA, 0x2);
	 eeprom_write_byte(0xEB, 0x0);
	 eeprom_write_byte(0xEC, 0xFF);
	 eeprom_write_byte(0xED, 0xFF);
	 eeprom_write_byte(0xEE, 0xFF);
	 eeprom_write_byte(0xEF, 0xFF);
	 eeprom_write_byte(0xF0, 0x0);
	 eeprom_write_byte(0xF1, 0x0);
	 eeprom_write_byte(0xF2, 0x0);
	 eeprom_write_byte(0xF3, 0x0);
	 eeprom_write_byte(0xF4, 0x0);
	 eeprom_write_byte(0xF5, 0x0);
	 eeprom_write_byte(0xF6, 0x0);
	 eeprom_write_byte(0xF7, 0x0);
	 eeprom_write_byte(0xF8, 0x0);
	 eeprom_write_byte(0xF9, 0x0);
	 eeprom_write_byte(0xFA, 0x0);
	 eeprom_write_byte(0xFB, 0x1);
	 eeprom_write_byte(0xFC, 0x1);
	 eeprom_write_byte(0xFD, 0x1);
	 eeprom_write_byte(0xFE, 0x1);
	 eeprom_write_byte(0xFF, 0x1);
	 eeprom_write_byte(0x100, 0x1);
	 eeprom_write_byte(0x101, 0x1);
	 eeprom_write_byte(0x102, 0x1);
	 eeprom_write_byte(0x103, 0x1);
	 eeprom_write_byte(0x104, 0x1);
	 eeprom_write_byte(0x105, 0x1);
	 eeprom_write_byte(0x106, 0x2);
	 eeprom_write_byte(0x107, 0x2);
	 eeprom_write_byte(0x108, 0x2);
	 eeprom_write_byte(0x109, 0x2);
	 eeprom_write_byte(0x10A, 0x2);
	 eeprom_write_byte(0x10B, 0x2);
	 eeprom_write_byte(0x10C, 0x2);
	 eeprom_write_byte(0x10D, 0x2);
	 eeprom_write_byte(0x10E, 0x2);
	 eeprom_write_byte(0x10F, 0x2);
	 eeprom_write_byte(0x110, 0x2);
	 eeprom_write_byte(0x111, 0x3);
	 eeprom_write_byte(0x112, 0x3);
	 eeprom_write_byte(0x113, 0x3);
	 eeprom_write_byte(0x114, 0x3);
	 eeprom_write_byte(0x115, 0x3);
	 eeprom_write_byte(0x116, 0x3);
	 eeprom_write_byte(0x117, 0x3);
	 eeprom_write_byte(0x118, 0x3);
	 eeprom_write_byte(0x119, 0x3);
	 eeprom_write_byte(0x11A, 0x3);
	 eeprom_write_byte(0x11B, 0x3);
	 eeprom_write_byte(0x11C, 0x4);
	 eeprom_write_byte(0x11D, 0x4);
	 eeprom_write_byte(0x11E, 0x4);
	 eeprom_write_byte(0x11F, 0x4);
	 eeprom_write_byte(0x120, 0x4);
	 eeprom_write_byte(0x121, 0x4);
	 eeprom_write_byte(0x122, 0x4);
	 eeprom_write_byte(0x123, 0x4);
	 eeprom_write_byte(0x124, 0x4);
	 eeprom_write_byte(0x125, 0x4);
	 eeprom_write_byte(0x126, 0x4);
	 eeprom_write_byte(0x127, 0x5);
	 eeprom_write_byte(0x128, 0xFF);
	 eeprom_write_byte(0x129, 0xFF);
	 eeprom_write_byte(0x12A, 0xFF);
	 eeprom_write_byte(0x12B, 0xFF);
	 */
}
