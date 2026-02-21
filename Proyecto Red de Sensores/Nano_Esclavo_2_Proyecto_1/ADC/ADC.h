#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>

// Inicializa el ADC con AVcc como referencia y prescaler de 128 (125kHz @ 16MHz)
void ADC_Init(void);

// Lee un canal específico (0-7) y devuelve un valor de 10 bits (0-1023)
uint16_t ADC_Read(uint8_t channel);

#endif