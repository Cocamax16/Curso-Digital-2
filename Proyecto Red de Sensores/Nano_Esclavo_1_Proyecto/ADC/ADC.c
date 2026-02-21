#include "ADC.h"
#include <util/delay.h>

void ADC_Init(void) {
	// REFS0 = 1: Usa AVcc (5V). REFS1 = 0: (Aseguramos que no use la interna de 1.1V)
	ADMUX = (1 << REFS0);
	
	// Habilitar ADC y Prescaler de 128 (Exactitud máxima)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_Read(uint8_t channel) {
	// Seleccionar canal limpiando bits previos
	ADMUX = (1 << REFS0) | (channel & 0x07);
	
	// Delay de estabilización para evitar saltos abruptos
	_delay_ms(2);

	// Iniciar conversión
	ADCSRA |= (1 << ADSC);

	// Esperar
	while (ADCSRA & (1 << ADSC));

	// Retornar valor de 10 bits
	return ADC;
}