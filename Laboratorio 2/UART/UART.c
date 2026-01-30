/*
 * UART.c
 *
 * Created: 1/27/2026 4:31:15 PM
 *  Author: Ususario
 */ 

#include "UART.h"
#define F_CPU 16000000


void UART_init(uint32_t baudrate) {
	uint16_t ubrr = F_CPU/16/baudrate - 1;
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Habilitar RX y TX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits, 1 stop bit
}

void UART_send_char(char data) {
	while (!(UCSR0A & (1 << UDRE0))); // Esperar a que el buffer esté vacío
	UDR0 = data;
}

void UART_send_string(char* str) {
	while (*str) UART_send_char(*str++);
}

char UART_receive_char(void) {
	while (!(UCSR0A & (1 << RXC0))); // Esperar a recibir datos
	return UDR0;
}

uint8_t UART_available(void) {
	return (UCSR0A & (1 << RXC0)); // Retorna 1 si hay algo para leer
}