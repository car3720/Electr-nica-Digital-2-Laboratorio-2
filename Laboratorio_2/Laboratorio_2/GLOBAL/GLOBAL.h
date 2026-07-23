/*
 * Global.h
 *
 * Created: 16/07/2026 19:00:35
 *  Author: Diego Cardona
 */ 


#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <avr/io.h>

// Resultados de los 4 canales del ADC (10 bits: 0-1023)
extern volatile uint16_t pot1;
extern volatile uint16_t pot2;
extern volatile uint16_t pot3;
extern volatile uint16_t pot4;

// Canal que se esta convirtiendo actualmente (0-3)
extern volatile uint8_t canal;

// Bandera para descartar la primera lectura tras cambiar de canal
extern volatile uint8_t dummy;

#endif