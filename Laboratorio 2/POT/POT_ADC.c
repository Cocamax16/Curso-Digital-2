/*
 * POT_ADC.c
 *
 * Created: 1/22/2026 9:05:12 PM
 *  Author: Ususario
 */ 

#include "POT_ADC.h"
#include <stdint.h>
#include <avr/io.h>

void adc_init() {
	
	DDRC &= ~((1 << PORTC2) | (1 << PORTC3));
	
	// Referencia de voltaje en AVcc (5V) con capacitor en pin AREF
	ADMUX = (1 << REFS0);
	
	// Habilitar ADC y configurar el pre-escalador a 128 (16MHz / 128 = 125KHz)
	// Esto asegura precisión en la conversión
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t canal) {
	// Limpiar los bits de selección de canal y seleccionar el nuevo canal
	// Se asegura de que el canal esté entre 0 y 7
	ADMUX = (ADMUX & 0xF0) | (canal & 0x07);
	
	// Iniciar la conversión
	ADCSRA |= (1 << ADSC);
	
	// Esperar a que la conversión termine (ADSC vuelve a 0)
	while (ADCSRA & (1 << ADSC));
	
	// Retornar el valor del registro ADC (10 bits)
	return ADC;
}
