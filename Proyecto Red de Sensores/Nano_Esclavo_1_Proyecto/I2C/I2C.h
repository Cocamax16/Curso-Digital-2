/*
 * I2C.h
 *
 * Created: 2/12/2026 9:28:21 AM
 *  Author: Ususario
 */ 

#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <stdint.h>

// Frecuencia de SCL = F_CPU / (16 + 2*TWBR * Prescaler)
#define SCL_F 100000UL // 100kHz

// Funciones Maestro
void I2C_Master_Init(void);
void I2C_Master_Start(void);
void I2C_Master_Stop(void);
uint8_t I2C_Master_Write(uint8_t data); // <--- Cambio aquí: devuelve uint8_t
uint8_t I2C_Master_Read(uint8_t ack);

// Funciones Esclavo
void I2C_Slave_Init(uint8_t address);
uint8_t I2C_Slave_Listen(void);
uint8_t I2C_Slave_Read(void);
void I2C_Slave_Write(uint8_t data);

#endif