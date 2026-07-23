/*
 * UART.h
 *
 * Created: 16/07/2026 19:03:23
 *  Author: Diego Cardona
 */ 

/*
 * uart.h
 *
 * Libreria basica de comunicacion UART para el ATmega328P.
 * IE3054 - Electronica Digital 2 - Laboratorio 2
 *
 * Usa el periferico de hardware USART0 (pines D0/RXD y D1/TXD).
 * La recepcion es por interrupcion, para no bloquear el loop principal
 * mientras se leen potenciometros y se actualiza el LCD.
 */

#ifndef UART_H
#define UART_H

#include <avr/io.h>

// Macro auxiliar para calcular UBRR a partir de F_CPU y el baud rate deseado
#define UART_UBRR(baud) ((F_CPU / 16UL / (baud)) - 1)

// Banderas de recepcion (actualizadas por la interrupcion USART_RX_vect)
extern volatile char uart_rx_char;
extern volatile uint8_t uart_rx_flag;

void uart_init(uint16_t ubrr);
void uart_transmit_char(char data);
void uart_transmit_string(const char *str);

// Regresa 1 si llego un caracter nuevo por UART, 0 si no
uint8_t uart_available(void);

// Devuelve el ultimo caracter recibido y limpia la bandera
char uart_read(void);

#endif