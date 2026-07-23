/*
 * UART.c
 *
 * Created: 16/07/2026 19:03:10
 *  Author: Diego Cardona
 */ 

/*
 * uart.c
 *
 * Implementacion de la libreria UART.
 * IE3054 - Electronica Digital 2 - Laboratorio 2
 */

#include <avr/interrupt.h>
#include "UART.h"

volatile char uart_rx_char = 0;
volatile uint8_t uart_rx_flag = 0;

void uart_init(uint16_t ubrr)
{
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;

    // Habilita transmisor, receptor e interrupcion de recepcion
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

    // 8 bits de datos, 1 bit de parada, sin paridad
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit_char(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));   // Espera a que el buffer este libre
    UDR0 = data;
}

void uart_transmit_string(const char *str)
{
    while (*str)
    {
        uart_transmit_char(*str);
        str++;
    }
}

uint8_t uart_available(void)
{
    return uart_rx_flag;
}

char uart_read(void)
{
    uart_rx_flag = 0;
    return uart_rx_char;
}

// Interrupcion de recepcion: se dispara cada vez que llega un byte nuevo
ISR(USART_RX_vect)
{
    uart_rx_char = UDR0;
    uart_rx_flag = 1;
}