/*
 * POT_ADC.h
 *
 * Created: 1/22/2026 9:07:44 PM
 *  Author: Ususario
 */ 


#ifndef POT_ADC_H_
#define POT_ADC_H_
#include <stdint.h>

void adc_init(void);

/**
 * @brief Lee el valor de un canal ADC específico.
 * @param canal El número del canal a leer (0-7). Para tu caso: 2 o 3.
 * @return Valor de 10 bits resultante de la conversión (0 a 1023).
 */
uint16_t adc_read(uint8_t canal);



#endif /* POT_ADC_H_ */