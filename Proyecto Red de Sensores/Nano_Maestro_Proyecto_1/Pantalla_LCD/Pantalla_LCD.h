#ifndef PANTALLA_LCD_H_
#define PANTALLA_LCD_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// NUEVA CONFIGURACIÓN: Control en PORTB
#define LCD_CTRL_PORT PORTB
#define LCD_CTRL_DDR  DDRB

#define RS PORTB2  // Pin Digital 10 en Nano
#define E  PORTB3  // Pin Digital 11 en Nano

void configuracion_LCD_8_bitas(void);
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_print(char *str);

#endif