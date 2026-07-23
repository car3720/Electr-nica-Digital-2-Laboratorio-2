/*
 * ADC.h
 *
 * Created: 18/05/2026 21:04:12
 *  Author: Diego Cardona
 */ 


#ifndef ADC_H_
#define ADC_H_
#include <avr/io.h>
void initADC(void);
void pauseADC(void);
void resumeADC(void);
#endif