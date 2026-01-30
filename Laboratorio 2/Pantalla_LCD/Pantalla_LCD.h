/*
 * Pantalla_LCD.h
 *
 * Created: 1/22/2026 10:58:10 AM
 *  Author: Ususario
 */ 


#ifndef PANTALLA_LCD_H_
#define PANTALLA_LCD_H_
#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

#define LCD_DATA_PORT PORTD
#define LCD_DATA_DDR  DDRD
#define LCD_CTRL_PORT PORTB
#define LCD_CTRL_DDR  DDRB

#define RS PINB0
#define E  PINB1

//Iniciar LCD en modo 8 bits
void configuracion_LCD_8_bitas(void);

void lcd_command(unsigned char cmd);

void lcd_data(unsigned char data);

void lcd_print(char *str);




#endif /* PANTALLA_LCD_H_ */