/*
 * Laboratorio 2: Pantalla LCD - IE3054 Electronica Digital 2
 *
 * Created: 16/07/2026 18:25:28
 * Author : Diego Cardona
 */ 

/*
 * main.c
 *
 
 *
 * Integra:
 *   Parte 1: Lectura de Pot1 (ADC0) y visualizacion en formato "X.XXV"
 *   Parte 2: Lectura de Pot2 (ADC1) y visualizacion en formato decimal (0-1023)
 *   Parte 3: Comunicacion UART (envia lecturas, recibe '+'/'-' para un contador)
 *
 * ADC (libreria propia, adaptada a 10 bits):
 *   - initADC() configura conversion continua por interrupcion, ciclando
 *     los canales ADC0-ADC3 (pot1-pot4). Solo usamos pot1 y pot2 aqui.
 *
 * Distribucion final de pines:
 *   LCD  DB0-DB5 -> PB0-PB5 (D8-D13)
 *   LCD  DB6-DB7 -> PD5-PD6 (D5-D6)
 *   LCD  RS/RW/E -> PD2/PD3/PD4
 *   Pot1 -> ADC0 (A0/PC0)
 *   Pot2 -> ADC1 (A1/PC1)
 *   Pot3, Pot4 -> ADC2, ADC3 (A2/A3) -- no usados en este laboratorio,
 *                 pero la libreria los sigue leyendo en su ciclo interno
 *   UART -> D0 (RX) / D1 (TX)  [periferico de hardware]
 *
 * Layout en pantalla (16x2):
 *   Fila 0: S1:X.XXV S2:1023
 *   Fila 1: S3:<contador>
 */

#define F_CPU 16000000UL
#define BAUD  9600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "LCD/LCD.h"
#include "ADC/ADC.h"
#include "GLOBAL/GLOBAL.h"
#include "UART/UART.h"

volatile int8_t contador = 0;

// Convierte una lectura ADC (0-1023, Vref=5V) al formato "X.XXV"
static void format_voltage(uint16_t adc_value, char *buffer)
{
    uint16_t centivolts = ((uint32_t)adc_value * 500UL) / 1023UL; // Hace la conversión de 0-500 
    uint8_t entero    = centivolts / 100; //Permite que los valores se muestren en enteros como tal
    uint8_t decimales = centivolts % 100; //Permite que los valores muestren valores decimales

    buffer[0] = '0' + entero; //Almacena el valor del primer entero 
    buffer[1] = '.'; //Separa el valor por medio de un punto
    buffer[2] = '0' + (decimales / 10); //Utiliza el valor de decimales para establecer el primer valor como definido
    buffer[3] = '0' + (decimales % 10); //Utiliza un porcentaje para definir los pequeños valores
    buffer[4] = 'V'; //Muestra el valor del voltaje
    buffer[5] = '\0'; //Permite mostrar un espacio entre carácteres
}

int main(void)
{
    char buffer[8];

    lcd_init();
    initADC();
    uart_init(UART_UBRR(BAUD)); //Inicia el uso de Baud Rate

    sei();   // Habilitar interrupciones globales 

    // --- Etiquetas fijas en pantalla ---
    lcd_set_cursor(0, 0); //Posiciona el cursor en la fila 1 y columna 1
    lcd_print("S1:");
    lcd_set_cursor(0, 9);
    lcd_print("S2:");
    lcd_set_cursor(1, 0);
    lcd_print("S3:");

    while (1)
    {
        // Los pot1/pot2 se actualizan solos en segundo plano por la ISR del ADC.
        // Los copiamos con interrupciones deshabilitadas brevemente para evitar leer un valor a medias (son variables de 16 bits en un microcontrolador de 8 bits).
        cli();
        uint16_t adc1 = pot1;
        uint16_t adc2 = pot2;
        sei();

        // ---------- Parte 1: Pot1 -> voltaje ----------
        format_voltage(adc1, buffer);
        lcd_set_cursor(0, 3);
        lcd_print(buffer);

        // ---------- Parte 2: Pot2 -> decimal (0-1023) ----------
        lcd_set_cursor(0, 12);
        lcd_print("    ");        // Limpia el campo (max 4 digitos)
        itoa(adc2, buffer, 10);
        lcd_set_cursor(0, 12);
        lcd_print(buffer);

        // ---------- Parte 3: Enviar lecturas por UART ----------
        itoa(adc1, buffer, 10);
        uart_transmit_string("POT1:");
        uart_transmit_string(buffer);
        itoa(adc2, buffer, 10);
        uart_transmit_string(" POT2:");
        uart_transmit_string(buffer);
        uart_transmit_string("\r\n");

        // ---------- Parte 3: Recibir '+' / '-' y actualizar contador ----------
        if (uart_available())
        {
            char c = uart_read();
            if (c == '+')
            {
                contador++;
            }
            else if (c == '-')
            {
                contador--;
            }
        }

        lcd_set_cursor(1, 3);
        lcd_print("    ");        // Limpia el campo del contador
        itoa(contador, buffer, 10);
        lcd_set_cursor(1, 3);
        lcd_print(buffer);

        _delay_ms(200);
    }

    return 0;
}

