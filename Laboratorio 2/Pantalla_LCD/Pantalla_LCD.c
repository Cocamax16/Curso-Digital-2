#include "Pantalla_LCD.h"
void configuracion_LCD_8_bitas(void) {
	// Configuración de puertos
	DDRD |= 0xFC; // PD2 a PD7 como salida
	DDRB |= (1 << DDB2) | (1 << DDB3); // PB2 y PB3 como salida (D0 y D1 del LCD)
	
	LCD_CTRL_DDR |= (1 << RS) | (1 << E);
	
	_delay_ms(50);
	
	lcd_command(0x30);
	_delay_ms(5);
	lcd_command(0x30);
	_delay_us(150);
	lcd_command(0x30);
	
	lcd_command(0x38);     // 8 bits, 2 líneas
	lcd_command(0x0C);     // Display ON
	lcd_command(0x06);
	lcd_command(0x01);     // Limpiar
	_delay_ms(2);
}

void lcd_command(unsigned char cmd) {
	// --- Mapeo de Bits ---
	// Bit 0 -> PINB2
	if(cmd & 0x01) PORTB |= (1 << PINB2); else PORTB &= ~(1 << PINB2);
	// Bit 1 -> PINB3
	if(cmd & 0x02) PORTB |= (1 << PINB3); else PORTB &= ~(1 << PINB3);
	
	// Bits 2 al 7 -> PIND2 al PIND7
	// Limpiamos los pines PD2-PD7 y cargamos los bits correspondientes de 'cmd'
	PORTD = (PORTD & 0x03) | (cmd & 0xFC);

	LCD_CTRL_PORT &= ~(1 << RS); // RS = 0 (Comando)
	
	_delay_ms(1);                // Estabilización
	LCD_CTRL_PORT |= (1 << E);   // E = 1
	_delay_ms(2);
	LCD_CTRL_PORT &= ~(1 << E);  // E = 0
	_delay_ms(2);
}

void lcd_data(unsigned char data) {
	// --- Mapeo de Bits ---
	if(data & 0x01) PORTB |= (1 << PINB2); else PORTB &= ~(1 << PINB2);
	if(data & 0x02) PORTB |= (1 << PINB3); else PORTB &= ~(1 << PINB3);
	
	PORTD = (PORTD & 0x03) | (data & 0xFC);

	LCD_CTRL_PORT |= (1 << RS);  // RS = 1 (Dato)
	
	_delay_ms(1);
	LCD_CTRL_PORT |= (1 << E);   // E = 1
	_delay_ms(2);
	LCD_CTRL_PORT &= ~(1 << E);  // E = 0
	_delay_ms(1);
}

void lcd_print(char *str) {
	while (*str) {          // Mientras no lleguemos al final de la cadena
		lcd_data(*str++);   // Envía el carácter actual y pasa al siguiente
	}
}