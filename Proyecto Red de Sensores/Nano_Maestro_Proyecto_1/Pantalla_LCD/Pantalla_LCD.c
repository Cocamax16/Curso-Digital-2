#include "Pantalla_LCD.h"

void configuracion_LCD_8_bitas(void) {
	// PD2 a PD7 como salida (Datos D0-D5)
	DDRD |= 0xFC;
	
	// PB0 a PB3 como salida:
	// PB0-PB1 (Datos D6-D7) y PB2-PB3 (Control RS-E)
	DDRB |= (1 << PORTB0) | (1 << PORTB1) | (1 << RS) | (1 << E);
	
	_delay_ms(50);
	lcd_command(0x30); _delay_ms(5);
	lcd_command(0x30); _delay_ms(1);
	lcd_command(0x30);
	
	lcd_command(0x38); // 8 bits, 2 líneas
	lcd_command(0x0C); // Display ON
	lcd_command(0x01); // Limpiar
	_delay_ms(2);
}

void lcd_command(unsigned char cmd) {
	// Repartir bits del comando hacia los puertos
	PORTD = (PORTD & 0x03) | (cmd << 2); // Bits 0-5 a PD2-PD7
	PORTB = (PORTB & 0xFC) | (cmd >> 6); // Bits 6-7 a PB0-PB1

	LCD_CTRL_PORT &= ~(1 << RS); // RS = 0 para comando
	LCD_CTRL_PORT |= (1 << E);
	_delay_ms(2);
	LCD_CTRL_PORT &= ~(1 << E);
	_delay_ms(2);
}

void lcd_data(unsigned char data) {
	PORTD = (PORTD & 0x03) | (data << 2);
	PORTB = (PORTB & 0xFC) | (data >> 6);

	LCD_CTRL_PORT |= (1 << RS); // RS = 1 para dato
	LCD_CTRL_PORT |= (1 << E);
	_delay_ms(2);
	LCD_CTRL_PORT &= ~(1 << E);
	_delay_ms(1);
}

void lcd_print(char *str) {
	while (*str) {
		lcd_data(*str++);
	}
}