/*
 * LCD.c
 *
 * Created: 16/07/2026 18:36:17
 *  Author: Diego Cardona
 */ 

/*
 * lcd.c
 *
 * Implementacion de la libreria LCD HD44780 en modo 8 bits.
 * IE3054 - Electronica Digital 2
 */

#include "LCD.h"

// ---------------------------------------------------------------
// Funciones privadas (uso interno de la libreria)
// ---------------------------------------------------------------

// Coloca un byte completo en el bus de datos (PORTB + PORTD)
static void lcd_set_data_bus(uint8_t data)
{
    // Bits 0-5 del dato van a PB0-PB5 (se conservan PB6/PB7 tal cual estan)
    PORTB = (PORTB & ~LCD_PORTB_MASK) | (data & LCD_PORTB_MASK);

    // Bit 6 del dato -> PD5, bit 7 del dato -> PD6
    // Se conservan los demas bits de PORTD (RS, RW, E, UART, etc.)
    PORTD = (PORTD & ~((1 << LCD_DB6_PIN) | (1 << LCD_DB7_PIN)))
          | (((data >> 6) & 0x01) << LCD_DB6_PIN)
          | (((data >> 7) & 0x01) << LCD_DB7_PIN);
}

// Genera el pulso de Enable necesario para que el LCD tome el dato
static void lcd_pulse_enable(void)
{
    PORTD |= (1 << LCD_E_PIN);
    _delay_us(1);              // PWEH minimo ~450ns, con margen
    PORTD &= ~(1 << LCD_E_PIN);
}

// ---------------------------------------------------------------
// Funciones publicas
// ---------------------------------------------------------------

void lcd_command(uint8_t cmd)
{
    PORTD &= ~(1 << LCD_RS_PIN);   // RS = 0 -> instruccion
    PORTD &= ~(1 << LCD_RW_PIN);   // R/W = 0 -> escritura

    lcd_set_data_bus(cmd);
    lcd_pulse_enable();

    _delay_us(50);   // La mayoria de instrucciones tardan 37us (margen)
}

void lcd_data(uint8_t data)
{
    PORTD |= (1 << LCD_RS_PIN);    // RS = 1 -> dato
    PORTD &= ~(1 << LCD_RW_PIN);   // R/W = 0 -> escritura

    lcd_set_data_bus(data);
    lcd_pulse_enable();

    _delay_us(50);
}

void lcd_clear(void)
{
    lcd_command(0x01);
    _delay_ms(2);   // Clear display tarda 1.64ms segun datasheet
}

void lcd_home(void)
{
    lcd_command(0x02);
    _delay_ms(2);   // Return home tambien tarda 1.64ms
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t address = (row == 0) ? col : (0x40 + col);
    lcd_command(0x80 | address);
}

void lcd_print(const char *str)
{
    while (*str)
    {
        lcd_data((uint8_t)(*str));
        str++;
    }
}

void lcd_create_char(uint8_t location, const uint8_t pattern[8])
{
    location &= 0x07;                  // Solo hay 8 localidades (0-7)
    lcd_command(0x40 | (location << 3)); // Set CGRAM address

    for (uint8_t i = 0; i < 8; i++)
    {
        lcd_data(pattern[i] & 0x1F);   // Solo 5 bits (columnas) son validos
    }
}

void lcd_init(void)
{
    // --- Configurar direcciones de pines como salida ---
    // PORTC NO se toca aqui: en este proyecto esos pines son las entradas
    // analogicas del ADC (ADC0-ADC3) y deben permanecer como entradas.
    DDRB |= LCD_PORTB_MASK;
    DDRD |= (1 << LCD_RS_PIN) | (1 << LCD_RW_PIN) | (1 << LCD_E_PIN)
          | (1 << LCD_DB6_PIN) | (1 << LCD_DB7_PIN);

    PORTD &= ~((1 << LCD_RS_PIN) | (1 << LCD_RW_PIN) | (1 << LCD_E_PIN));

    _delay_ms(20);   // Esperar >15ms despues de encendido (VCC sube a 4.5V)

    // --- Secuencia de inicializacion para 8 bits ---
    PORTD &= ~(1 << LCD_RS_PIN);
    PORTD &= ~(1 << LCD_RW_PIN);

    lcd_set_data_bus(0x30);   // Function set (8 bits) - primer intento
    lcd_pulse_enable();
    _delay_ms(5);             // Esperar >4.1ms

    lcd_pulse_enable();       // Function set - segundo intento (mismo dato)
    _delay_us(150);           // Esperar >100us

    lcd_pulse_enable();       // Function set - tercer intento
    _delay_us(150);

    // --- A partir de aqui se usan comandos normales ---
    lcd_command(0x38);   // 8 bits, 2 lineas, fuente 5x8
    lcd_command(0x08);   // Display off
    lcd_clear();          // Clear display
    lcd_command(0x06);   // Entry mode: incrementa, sin shift
    lcd_command(0x0C);   // Display on, cursor off, blink off
}