/*
 * LCD.h
 *
 * Created: 16/07/2026 18:36:27
 *  Author: Diego Cardona
 */ 

/*

 *
 * Libreria para manejo de LCD HD44780 en modo de 8 bits.
 * IE3054 - Electronica Digital 2
 *
 * Distribucion de pines (ATmega328P / Arduino Nano):
 *   DB0-DB5 -> PB0-PB5 (D8-D13)
 *   DB6-DB7 -> PD5-PD6 (D5-D6)
 *   RS      -> PD2
 *   R/W     -> PD3
 *   E       -> PD4
 *
 * Nota: PB6 y PB7 del ATmega328P estan conectados al cristal externo
 * de 16MHz en el Arduino Nano (XTAL1/XTAL2), por lo que NO estan
 * disponibles como pines digitales. Por eso el bus se completa con
 * dos pines de PORTD en vez de PORTB.
 *
 * PORTC se deja completamente libre (sin configurar como salida) porque
 * en este laboratorio se usa para los canales ADC0-ADC3 (potenciometros).
 */

#ifndef LCD_H
#define LCD_H

// F_CPU debe estar definido ANTES de <util/delay.h> en CADA archivo .c que
// use _delay_ms()/_delay_us(), porque cada .c se compila por separado y no
// hereda el #define F_CPU de main.c. Si no se define, avr-libc asume 1MHz
// por defecto y todos los delays de esta libreria quedarian ~16 veces mas
// cortos de lo necesario, rompiendo la inicializacion del LCD.
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

// --- Pines de control (PORTD) ---
#define LCD_RS_PIN   PD2
#define LCD_RW_PIN   PD3
#define LCD_E_PIN    PD4

// --- Pines de datos altos (PORTD) ---
#define LCD_DB6_PIN  PD5
#define LCD_DB7_PIN  PD6

// --- Mascara de datos bajos (PORTB) ---
#define LCD_PORTB_MASK 0x3F   // PB0-PB5 (DB0-DB5)

// Inicializa el LCD en modo 8 bits, 2 lineas, fuente 5x8
void lcd_init(void);

// Envia un byte de comando (RS = 0)
void lcd_command(uint8_t cmd);

// Envia un byte de dato/caracter (RS = 1)
void lcd_data(uint8_t data);

// Borra la pantalla y regresa el cursor a home
void lcd_clear(void);

// Regresa el cursor a la posicion inicial (sin borrar contenido)
void lcd_home(void);

// Posiciona el cursor: fila (0 o 1), columna (0-15 para display 16x2)
void lcd_set_cursor(uint8_t row, uint8_t col);

// Imprime una cadena de texto terminada en '\0'
void lcd_print(const char *str);

// Define un caracter personalizado en CGRAM (location: 0-7, pattern: 8 bytes de 5 bits)
void lcd_create_char(uint8_t location, const uint8_t pattern[8]);

#endif