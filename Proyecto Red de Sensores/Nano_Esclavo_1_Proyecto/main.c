#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "I2C/I2C.h"

#define MY_ADDR 0x20
#define PIN_IR PD2
#define PIN_BOTON PD3
#define IN1 PD4
#define IN2 PD5
#define IN3 PD6
#define IN4 PD7

volatile uint8_t estado_ir = 0;
volatile uint8_t estado_btn = 0;

void mover_stepper() {
	uint8_t pasos[] = { (1<<IN1), (1<<IN1)|(1<<IN2), (1<<IN2), (1<<IN2)|(1<<IN3),
	(1<<IN3), (1<<IN3)|(1<<IN4), (1<<IN4), (1<<IN4)|(1<<IN1) };
	for(int i=0; i<8; i++) {
		PORTD = (PORTD & 0x0F) | (pasos[i] & 0xF0);
		_delay_ms(1);
	}
}

ISR(TWI_vect) {
	uint8_t status = (TWSR & 0xF8);
	switch (status) {
		case 0xA8:
		TWDR = estado_ir;
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
		break;
		case 0xB8:
		TWDR = estado_btn;
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
		break;
		default:
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
		break;
	}
}

int main(void) {
	DDRD &= ~((1 << PIN_IR) | (1 << PIN_BOTON));
	PORTD |= (1 << PIN_IR) | (1 << PIN_BOTON);
	DDRD |= (1 << IN1) | (1 << IN2) | (1 << IN3) | (1 << IN4);
	I2C_Slave_Init(MY_ADDR);
	TWCR |= (1 << TWIE);
	sei();
	
	while(1) {
		estado_ir = (PIND & (1 << PIN_IR)) ? 0 : 1;
		estado_btn = (PIND & (1 << PIN_BOTON)) ? 0 : 1;
		if (estado_btn) mover_stepper();
		else PORTD &= 0x0F;
	}
}