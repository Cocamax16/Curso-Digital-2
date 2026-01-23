/*
 * Display.c
 *
 * Created: 1/15/2026 7:25:23 PM
 *  Author: Ususario
 */ 

#include <avr/io.h>
#include "Display.h"

void LedsDisplay(uint8_t Numero){
	switch(Numero){
		case 0:
		PORTD = 0x00;
		break;
		case 1:
		PORTD = 0x3F;
		break;
		case 2:
		PORTD = 0x06;
		break;
		case 3:
		PORTD = 0x5B;
		break;
		case 4:
		PORTD = 0x4F;
		break;
		case 5:
		PORTD = 0x66;
		break;
		case 6:
		PORTD = 0x6D;
		break;
		case 7:
		PORTD = 0x7D;
		break;
		case 8:
		PORTD = 0x07;
		break;
		case 9:
		PORTD = 0x7F;
		break;
		case 10:
		PORTD = 0x6F;
		break;
	}
	
}
