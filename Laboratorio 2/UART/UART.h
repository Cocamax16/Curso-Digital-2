/*
 * UART.h
 *
 * Created: 1/27/2026 4:32:01 PM
 *  Author: Ususario
 */ 



#ifndef UART_H_
#define UART_H_

#include <avr/io.h>

void UART_init(uint32_t baudrate);
void UART_send_char(char data);
void UART_send_string(char* str);
char UART_receive_char(void);
uint8_t UART_available(void);

#endif