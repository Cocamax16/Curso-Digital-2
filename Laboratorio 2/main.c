/*
 * Laboratorio_2_Pantalla_LCD.c
 *
 * Created: 1/22/2026 10:53:32 AM
 * Author : Ususario
 */ 

#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>

#include "Pantalla_LCD/Pantalla_LCD.h"
#include "POT/POT_ADC.h"
#include "UART/UART.h"
#include <stdint.h>
#include <stdio.h>
uint16_t pot1, pot2;
char buffer[16];
uint16_t adc_s1, adc_s2;
uint32_t volt_s1, volt_s2;
#include "UART/UART.h" // Asegúrate de incluir tu nueva librería

int main(void) {
	configuracion_LCD_8_bitas();
	adc_init();
	UART_init(9600); // Inicializar a 9600 baudios
	
	int16_t contador_s3 = 0; // Cambiamos a int para que no haya problemas al decrementar
	lcd_command(0x01);
	_delay_ms(2);

	while(1) {
		// 1. Leer ADC
		adc_s1 = adc_read(2);
		adc_s2 = adc_read(3);
		volt_s1 = (adc_s1 * 5000UL) / 1023UL;

		// 2. Revisar si hay datos en la consola (UART)
		if (UART_available()) {
			char tecla = UART_receive_char();
			if (tecla == '+') {
				contador_s3++; // Incrementar con '+'
				} else if (tecla == '-') {
				contador_s3--; // Decrementar con '-'
			}
			

		}

		// 3. Actualizar LCD
		lcd_command(0x80);
		lcd_print("S1:   S2:   S3: ");

		lcd_command(0xC0);
		// S2 se muestra en decimal (0-1023)
		sprintf(buffer, "%ld.%02ldV %4u  %3d",
		volt_s1/1000, (volt_s1%1000)/10,
		adc_s2,
		contador_s3);

		lcd_print(buffer);
		_delay_ms(50); // Reducimos el delay para que la UART sea más responsiva
		
		// Enviar datos de vuelta a la PC
		sprintf(buffer, "S1:%ld.%02ldV S2:%u\r\n", volt_s1/1000, (volt_s1%1000)/10, adc_s2);
		UART_send_string(buffer);
		_delay_ms(50);
	}
}

