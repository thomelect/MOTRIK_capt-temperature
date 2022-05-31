/**
 * @file 	 adc.c
 * @brief 	 Librairie ADC qui comprend les fonctions d'initialisation et de transmission de la valeur lu vers le main (getter). La converssion se déclanche manuellement larsque l'on appel la fonction adcGetValue().
 * @author 	 Thomas Desrosiers
 * @version  1.1
 * @date 	 2022/02/11
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"

uint16_t _adcVal[2];

void adcInit()
{
	// ADMUX = REFS1 REFS0 ADLAR MUX4 MUX3 MUX2 MUX1 MUX0
	// ADCSRA = ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS0
	// ADCSRB = ADHSM ACME MUX5 – ADTS3 ADTS2 ADTS1 ADTS0
	// DIDR0 = ADC7D ADC6D ADC5D ADC4D - - ADC1D ADC0D
	// DIDR2 = - - ADC13D ADC12D ADC11D ADC10D ADC9D ADC8D
	ADMUX = (1 << REFS0);								   // Vcc Voltage Reference | ADC Right Adjust Result | Input ADC0.
	ADCSRA = (1 << ADEN) | (1 << ADPS2) /* | (1<<ADPS1)*/; // ADC Enable | ADC Prescaler \16.
	ADCSRB = (1 << ADHSM);								   // ADC High Speed Mode.
	DIDR0 = 0b00000001; // entrée numérique désactivée pour le ADC0 / ADC1.
	//DIDR2 = 0b00000011; // entrée numérique désactivée pour le ADC8 / ADC9.
}

uint16_t adcGetValue(uint8_t canal)
{
	ADMUX &= ~1; // ADC0 - CAPT1
	ADCSRB &= ~0b00100000; // ADC0 - CAPT1
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));	   // Ligne qui permet d'attendre qui la mesure soit terminée.
	_adcVal[0] = (ADCL | (ADCH << 8)); // Mesure de ADC0 - CAPT1
	//ADMUX |= 0b00100000;				   // ADC8 - CAPT2
	ADCSRB |= 0b00100000;				   // ADC8 - CAPT2
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));	   // Ligne qui permet d'attendre qui la mesure soit terminée.
	_adcVal[1] = (ADCL | (ADCH << 8)); // Mesure de ADC8 - CAPT2
	ADMUX &= ~1; // ADC0 - CAPT1
	ADMUX |= 0b00000001;				   // ADC9 - CAPT3
	ADCSRB |= 0b00100000;				   // ADC9 - CAPT3
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));	   // Ligne qui permet d'attendre qui la mesure soit terminée.
	_adcVal[2] = (ADCL | (ADCH << 8)); // Mesure de ADC9 - CAPT3
	return _adcVal[canal];			   // Retourne la valeur de ADC 0, 8 ou 9 en fopnction de la valeur de canal.
}