/*
 * I2C.c
 *
 * Created: 2/12/2026 9:28:12 AM
 *  Author: Ususario
 */ 

#include "I2C.h"
#define F_CPU 16000000UL
// ===== FUNCIONES MAESTRO =====
void I2C_Master_Init(void) {
	// Activar resistencias pull-up internas en A4 y A5
	PORTC |= (1 << PORTC4) | (1 << PORTC5);
	
	TWSR = 0x00; // Prescaler = 1
	TWBR = ((F_CPU / SCL_F) - 16) / 2;
	TWCR = (1 << TWEN);
}

void I2C_Master_Start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void I2C_Master_Stop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	while (TWCR & (1 << TWSTO)); // Espera a que termine el STOP
}

uint8_t I2C_Master_Write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	
	uint16_t timeout = 10000;
	while (!(TWCR & (1 << TWINT))) {
		if (--timeout == 0) return 1; // Error por tiempo agotado
	}
	
	// Verificar si recibimos ACK (Estado 0x18 para SLA+W o 0x28 para data)
	uint8_t status = TWSR & 0xF8;
	if (status != 0x18 && status != 0x28 && status != 0x40) return 1;
	
	return 0; // Todo bien
}

uint8_t I2C_Master_Read(uint8_t ack) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (ack << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

// ===== FUNCIONES ESCLAVO =====
void I2C_Slave_Init(uint8_t address) {
	TWAR = (address << 1); // El bit 0 es para el llamado general, lo dejamos en 0
	TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
}

uint8_t I2C_Slave_Listen(void) {
	while (!(TWCR & (1 << TWINT)));
	return (TWSR & 0xF8); // Retorna el estado del bus
}

uint8_t I2C_Slave_Read(void) {
	TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

void I2C_Slave_Write(uint8_t data) {
	TWDR = data;
	// Debemos habilitar el ACK (TWEA) para que el esclavo pueda seguir recibiendo/transmitiendo
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
}