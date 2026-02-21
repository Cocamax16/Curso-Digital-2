#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C/I2C.h"
#include "ADC/ADC.h"

#define MY_ADDR 0x21
#define IN1 PD7
#define IN2 PB0 // Pin D8
#define ENA PD6 // Pin D6 (PWM)

volatile uint16_t valor_adc = 0;
volatile uint8_t boton_presionado = 0;

void Motores_Init() {
	DDRD |= (1 << IN1) | (1 << ENA);
	DDRB |= (1 << IN2) | (1 << PB1); // Servo en D9

	// Timer 1: Servo
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	ICR1 = 39999;

	// Timer 0: Motor DC PWM
	TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
	TCCR0B = (1 << CS01) | (1 << CS00);
}

ISR(TWI_vect) {
	uint8_t status = (TWSR & 0xF8);
	switch (status) {
		case 0xA8: // Maestro pide datos (envía ADC)
		TWDR = (uint8_t)(valor_adc & 0xFF);
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
		break;
		case 0xB8: // Maestro pide segundo byte
		TWDR = (uint8_t)(valor_adc >> 8);
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
		break;
		case 0x80: // Maestro ENVÍA datos (Estado del botón)
		boton_presionado = TWDR;
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
		break;
		default:
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
		break;
	}
}

int main(void) {
	ADC_Init();
	Motores_Init();
	I2C_Slave_Init(MY_ADDR);
	TWCR |= (1 << TWIE);
	sei();
	
	while(1) {
		valor_adc = ADC_Read(0);
		
		// El Servo siempre sigue al Potenciómetro
		OCR1A = 1000 + ((uint32_t)valor_adc * 4000 / 1023);
		
		// El Motor DC solo se mueve si el botón (leído por el maestro) está en 1
		if (boton_presionado) {
			PORTD |= (1 << IN1);
			PORTB &= ~(1 << IN2);
			OCR0A = valor_adc / 4; // Velocidad según Pot
			} else {
			PORTD &= ~(1 << IN1);
			PORTB &= ~(1 << IN2);
			OCR0A = 0; // Motor apagado
		}
		_delay_ms(30);
	}
}