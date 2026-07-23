/*
 * ADC.c
 * Módulo que configura y maneja el Convertidor Analógico-Digital (ADC) del ATmega328P.
 * Lee 4 potenciómetros conectados a ADC0-ADC3 de forma cíclica usando interrupciones,
 * sin bloquear el procesador principal.
 *
 * Created: 18/05/2026 21:04:03
 *  Author: Diego Cardona
 */

#include "ADC.h"               // Cabecera de este módulo (prototipos de funciones)
#include "../GLOBAL/GLOBAL.h"  // Variables globales: pot1-pot4, canal, dummy
#include <avr/interrupt.h>     // Macro ISR() para definir rutinas de interrupción

// =============================================================================
// initADC() — Configura el ADC antes de empezar a leer
// =============================================================================
void initADC(void)
{
	// ?? Registro ADMUX: selecciona referencia de voltaje y canal inicial ??????

	ADMUX = 0;                  // Limpia el registro ADMUX (partimos de cero)
	ADMUX |= (1<<REFS0);        // REFS0=1, REFS1=0 ? referencia AVcc (~5 V)
	// El ADC mide voltajes entre 0 V y AVcc

	// NOTA: a diferencia de la version anterior, aqui NO se activa ADLAR.
	// Este laboratorio (IE3054 Lab 2) pide desplegar el ADC en su rango
	// completo de 0-1023, por lo que necesitamos los 10 bits del resultado
	// (ADCL + ADCH), no solo los 8 bits altos de ADCH.

	// MUX[3:0] = 0000 ? Canal ADC0 seleccionado por defecto (ya que ADMUX=0 al inicio)

	// ?? Deshabilitar buffers digitales en los pines ADC0-ADC3 ?????????????????
	// Cuando un pin se usa como entrada analógica, el buffer digital consume corriente
	// innecesariamente y puede introducir ruido. Deshabilitarlo mejora la precisión.
	DIDR0 |= (1<<ADC0D) |       // Deshabilita buffer digital en ADC0 (PC0)
	(1<<ADC1D) |      // Deshabilita buffer digital en ADC1 (PC1)
	(1<<ADC2D) |      // Deshabilita buffer digital en ADC2 (PC2)
	(1<<ADC3D);       // Deshabilita buffer digital en ADC3 (PC3)

	// ?? Registro ADCSRA: control principal del ADC ????????????????????????????

	ADCSRA = 0;                 // Limpia el registro de control del ADC

	ADCSRA |= (1<<ADEN);        // ADEN=1 ? HABILITA el ADC
	// Sin esto, el ADC no funciona

	ADCSRA |= (1<<ADIE);        // ADIE=1 ? Habilita la INTERRUPCIÓN del ADC
	// Cuando termina una conversión, se llama ISR(ADC_vect)
	// Esto permite que el CPU haga otras cosas mientras convierte

ADCSRA |= (1<<ADPS2) |      // ADPS2=1 }
(1<<ADPS1) |      // ADPS1=1 }? Prescaler = 128
(1<<ADPS0);       // ADPS0=1 }
// F_CPU=16MHz / 128 = 125 kHz para el ADC
// El ADC necesita entre 50-200 kHz para máxima precisión

// ?? Arranque inicial con lectura "dummy" ??????????????????????????????????
// La primera conversión tras encender el ADC siempre se descarta:
// el capacitor interno aún no está estabilizado.
dummy = 1;                  // Marcamos que la próxima lectura es dummy (se descartará)
ADCSRA |= (1<<ADSC);        // ADSC=1 ? INICIA la primera conversión
// Al terminar, la ISR(ADC_vect) se ejecutará automáticamente
}

// =============================================================================
// pauseADC() — Detiene las interrupciones del ADC
// Se usa al entrar en modo Adafruit/UART para que el ADC no interfiera
// con la comunicación serial (ambos usan interrupciones y pueden colisionar)
// =============================================================================
void pauseADC(void)
{
	ADCSRA &= ~(1<<ADIE);       // ADIE=0 ? Deshabilita la interrupción ADC
	// El ADC sigue encendido pero ya no interrumpe al CPU
	ADCSRA &= ~(1<<ADSC);       // ADSC=0 ? Detiene cualquier conversión en curso
}

// =============================================================================
// resumeADC() — Reactiva el ADC después de haber sido pausado
// Se usa al salir del modo Adafruit y volver a Manual o EEPROM
// =============================================================================
void resumeADC(void)
{
	ADCSRA |= (1<<ADIE);        // Vuelve a habilitar la interrupción ADC
	dummy = 1;                  // La primera lectura tras reanudar se descarta
	// (el canal puede haber cambiado mientras estaba pausado)
	ADCSRA |= (1<<ADSC);        // Inicia una nueva conversión para arrancar el ciclo
}

// =============================================================================
// ISR(ADC_vect) — Rutina de Interrupción del ADC
// Se ejecuta AUTOMÁTICAMENTE cada vez que el ADC termina una conversión.
// No la llamas tú; la llama el hardware.
// Su trabajo: leer el resultado, guardarlo en pot1-pot4, avanzar al siguiente canal.
// =============================================================================
ISR(ADC_vect)
{
	// ?? Filtro de lectura dummy ???????????????????????????????????????????????
	// Después de cambiar de canal, la primera conversión puede ser imprecisa
	// (el multiplexor interno necesita tiempo para estabilizarse).
	// Si dummy=1, descartamos este resultado y pedimos una nueva conversión.
	if(dummy)
	{
		dummy = 0;              // Limpiamos la bandera: la SIGUIENTE lectura sí será válida
		ADCSRA |= (1<<ADSC);   // Iniciamos otra conversión inmediatamente
		return;                 // Salimos sin guardar nada
	}

	// ?? Ciclo de lectura de los 4 canales ?????????????????????????????????????
	// "canal" indica qué potenciómetro acabamos de convertir.
	// Leemos ADC (registro de 16 bits: ADCL + ADCH, 10 bits utiles) y cambiamos
	// al siguiente canal.
	// IMPORTANTE: se debe leer ADCL antes que ADCH (el compilador ya lo hace
	// correctamente al usar la macro "ADC", que internamente lee ambos en el
	// orden correcto).

	if(canal == 0)
	{
		pot1 = ADC;                         // Guardamos la lectura del canal 0 (ADC0) en pot1
		ADMUX = (ADMUX & 0xF0) | 0x01;     // Seleccionamos el canal 1 para la siguiente conversión
		// (ADMUX & 0xF0) conserva REFS0, solo cambia MUX
		canal = 1;                          // Actualizamos el rastreador de canal
	}
	else if(canal == 1)
	{
		pot2 = ADC;                         // Guardamos lectura del canal 1 (ADC1) en pot2
		ADMUX = (ADMUX & 0xF0) | 0x02;     // Seleccionamos canal 2
		canal = 2;
	}
	else if(canal == 2)
	{
		pot3 = ADC;                         // Guardamos lectura del canal 2 (ADC2) en pot3
		ADMUX = (ADMUX & 0xF0) | 0x03;     // Seleccionamos canal 3
		canal = 3;
	}
	else
	{
		pot4 = ADC;                         // Guardamos lectura del canal 3 (ADC3) en pot4
		ADMUX = (ADMUX & 0xF0) | 0x00;     // Volvemos al canal 0 ? ciclo continuo
		canal = 0;
	}

	// ?? Preparar la siguiente conversión ??????????????????????????????????????
	dummy = 1;              // La primera lectura del nuevo canal se descartará
	ADCSRA |= (1<<ADSC);   // Iniciamos la conversión del nuevo canal
	// Esto crea un ciclo infinito: 0?1?2?3?0?1?2?3?...
	// Los valores pot1-pot4 se actualizan continuamente
}