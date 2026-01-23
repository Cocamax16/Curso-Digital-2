#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "Configuracion/Configuracion.h"
#include "Display/Display.h"

// VOLATILE es fundamental para que el main vea los cambios en las ISR
volatile int cuenta_regresiva = -1;
volatile int juego = 1;         // 1: Bloqueado/Cuenta Regresiva, 0: Carrera Activa, 2: Fin de juego
volatile int jugador_1 = 0;     // Pulsos (0-10)
volatile int pos_led_1 = 0;     // Posición del LED (0-3)
volatile int jugador_2 = 0;     // Pulsos (0-10)
volatile int pos_led_2 = 0;     // Posición del LED (0-3)

int main(void) {
setup_puertos();
setup_interrupcion();
setup_timer1();

while (1) {
	if (juego == 0) { // Solo procesar si la carrera está activa
		
		// Lógica Jugador 1
		if (jugador_1 >= 10) {
			PORTB = (PORTB & 0xF0) | (1 << pos_led_1); // Desplazamiento 1, 2, 4, 8
			pos_led_1++;
			jugador_1 = 0;
			}

		// Lógica Jugador 2
		if (jugador_2 >= 10) {
			PORTC = (PORTC & 0xF0) | (1 << pos_led_2);
			pos_led_2++;
			jugador_2 = 0;
			}

			// DETECCIÓN DE GANADOR
		if (pos_led_1 > 3 || pos_led_2 > 3) {
			juego = 2; // Detener carrera
			if (pos_led_1 > 3) {
				PORTB |= 0x0F;
				PORTC &= 0xF0;
				LedsDisplay(2);
			} 
			else{
				PORTC |= 0x0F;
				PORTB &= 0xF0;
				LedsDisplay(3);
			}
		}
	}
}
}

// BOTÓN DE INICIO (Puerto B4)
ISR(PCINT0_vect) {
	if (!(PINB & (1 << PINB4))) {
		if (juego != 0 && cuenta_regresiva == -1) { // Solo si no hay carrera o conteo activo
			
			// Reiniciar todo para nueva carrera
			jugador_1 = 0; 
			jugador_2 = 0;
			pos_led_1 = 0; 
			pos_led_2 = 0;
			PORTB &= 0xF0; 
			PORTC &= 0xF0;	
			juego = 1; // Bloquea botones
			cuenta_regresiva = 5; // Inicia en 5
			LedsDisplay(6);
			start_timer1();
		}
	}
}

// BOTONES DE JUGADORES (Puerto C4 y C5)
ISR(PCINT1_vect) {
	if (juego == 0) {
		
		// Jugador 1
		if (!(PINC & (1 << PINC4))) {
			_delay_ms(20);
			if (!(PINC & (1 << PINC4))) {
				jugador_1++;
				while(!(PINC & (1 << PINC4)) && jugador_1 < 10);
			}
		}
		
		// Jugador 2
		if (!(PINC & (1 << PINC5))) {
			_delay_ms(20);
			if (!(PINC & (1 << PINC5))) {
				jugador_2++;
				while(!(PINC & (1 << PINC5)) && jugador_2 < 10);
			}
		}
	}
}

// TIMER PARA CUENTA REGRESIVA
ISR(TIMER1_COMPA_vect) {
	if (cuenta_regresiva > 0) {
		cuenta_regresiva--;
		LedsDisplay(cuenta_regresiva + 1);
		} else {
		stop_timer1();
		LedsDisplay(0);
		juego = 0;      // ¡ARRANCA LA CARRERA!
		cuenta_regresiva = -1;
	}
}
