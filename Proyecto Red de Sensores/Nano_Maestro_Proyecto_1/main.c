#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "I2C/I2C.h"
#include "Pantalla_LCD/Pantalla_LCD.h"

#define ADDR_0x20 0x20
#define ADDR_0x21 0x21

uint16_t s2_pot = 0;
uint8_t  s3_ir = 0;
uint8_t  s4_boton = 0;
char buffer_lcd[17];

// Función para enviar un byte a un esclavo
void enviar_dato_i2c(uint8_t address, uint8_t dato) {
	I2C_Master_Start();
	I2C_Master_Write(address << 1); // Modo Escritura
	I2C_Master_Write(dato);
	I2C_Master_Stop();
}

uint16_t pedir_datos_i2c(uint8_t address) {
	I2C_Master_Start();
	I2C_Master_Write((address << 1) | 0x01);
	uint8_t lo = I2C_Master_Read(1);
	uint8_t hi = I2C_Master_Read(0);
	I2C_Master_Stop();
	return ((uint16_t)hi << 8) | lo;
}

int main(void) {
	configuracion_LCD_8_bitas();
	I2C_Master_Init();
	lcd_command(0x01);
	_delay_ms(100);

	while(1) {
		// 1. Leer Esclavo 0x20 (Botón e IR)
		uint16_t raw_20 = pedir_datos_i2c(ADDR_0x20);
		if (raw_20 != 0xFFFF) {
			s3_ir = raw_20 & 0xFF;
			s4_boton = (raw_20 >> 8) & 0xFF;
		}

		_delay_ms(5);

		// 2. Enviar estado del botón al Esclavo 0x21
		enviar_dato_i2c(ADDR_0x21, s4_boton);

		_delay_ms(5);

		// 3. Leer Potenciómetro del Esclavo 0x21
		uint16_t raw_21 = pedir_datos_i2c(ADDR_0x21);
		if (raw_21 != 0xFFFF) s2_pot = raw_21 & 0x03FF;

		// Mostrar en LCD
		lcd_command(0x80);
		sprintf(buffer_lcd, "POT:%4u IR:%u ", s2_pot, s3_ir);
		lcd_print(buffer_lcd);

		lcd_command(0xC0);
		sprintf(buffer_lcd, "BTN:%u MOTOR:%s ", s4_boton, (s4_boton ? "ON " : "OFF"));
		lcd_print(buffer_lcd);

		_delay_ms(50);
	}
}