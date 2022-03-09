/**
 * @file 	  main.c
 * @brief 	  Partie du projet Motrik qui sera utilisé afin de mesurer la température avec le LM65.
 * @author 	  Thomas Desrosiers
 * @version   2.0
 * @date 	  2022/02/11

 * @mainpage  MOTRIK_capt-temperature
 * @author 	  Thomas Desrosiers
 * @section   MainSection1 Description
			  Partie du projet Motrik qui sera utilisé afin de mesurer la température avec le LM65.
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h>
#include "adc.h"
#include "usart.h"

#define FILTRE_SIZE 5

volatile uint8_t cntCinqCentMs = 0;
volatile uint8_t cinqCentMSFlag = 0;
volatile uint8_t refreshMesure = 0;
char msgTemp[17];
uint16_t adcValTemp = 0;
float temperatureAcquisition = 0;
float temperatureFiltered = 0;
uint8_t tblFlag = 0;
float tblData[FILTRE_SIZE];
uint8_t tblIndex = 0;

// Prototypes des fonctions locales
/**
 * @brief  Fonction d'initialisation du timer 0 avec une période de 4ms.
 */
void timer1Init();

/**
 *@brief  Fonction qui regroupe l'initialisation des différents I/O et des librairies.
 */
void miscInit(void);

float filtreFenetre(float tempRaw);

int main(void)
{
	miscInit();
	while (1)
	{
		if (refreshMesure) // Flag qui est vrai à chaque quatre ms.
		{
			refreshMesure = 0;
			adcValTemp = adcGetValue(0); // Lecture du canal 1 du ADC.
		}
		if (cinqCentMSFlag) // Flag qui est vrai à chaque cinq cent ms.
		{
			cinqCentMSFlag = 0;
			
			temperatureFiltered = filtreFenetre(temperatureAcquisition);
			sprintf(msgTemp, "Temperature: %0.1f  %0.1f\n\r", temperatureAcquisition, temperatureFiltered); // Conversion de la mesure de température en string.
			usartSendString(msgTemp);
		}
		temperatureAcquisition = (float)adcValTemp / 8.903512712; // Puisque le capteur de température est un capteur linéaire, 22 à été obtenu en prenant une valeur en début et en la divisant par un nombre qui nous permet d'arriver à la température actuelle.
	}
}

/**
 *@brief  Le timer 1 est initialisé à 4ms. à chaques 4ms, refresh mesure est HAUT et après 500ms(125 x 4ms) cinqCentMSFlag est HAUT.
 */
ISR(TIMER1_COMPA_vect)
{
	refreshMesure = 1; //À chaque 4ms. Ce flag sera utilisé pour faire une nouvelle mesure d'ADC.
	cntCinqCentMs++;
	if (cntCinqCentMs >= 125)
	{
		cntCinqCentMs -= 125;
		cinqCentMSFlag = 1; //À chaque 500ms. Ce flag sera utilisé pour changer les valeurs envoyés par les capteurs.
	}
}

void miscInit(void)
{
	timer1Init(); // Initialisation du timers #1.
	adcInit();	  // Initialisation du adc.
	usartInit(1000000, F_CPU);
	for (uint8_t i = 0; i < FILTRE_SIZE; i++)
	{
		tblData[i] = 0;
	}
}

void timer1Init()
{
	// TCCR1A : COM1A1 COM1A0 COM1B1 COM1B0 COM1C1 COM1C0 WGM11 WGM10
	// TCCR1B: ICNC1 ICES1 – WGM13 WGM12 CS12 CS11 CS10
	// TIMSK1: – – ICIE1 – OCIE1C OCIE1B OCIE1A TOIE1
	TCCR1B = (1 << WGM12);	 // mode CTC.
	TCCR1B |= (1 << CS12);	 // Prescaler de 256.
	TIMSK1 |= (1 << OCIE1A); // Output Compare A Match Interrupt Enable
	OCR1A = 250 - 1;		 // 62.5ns * 256 * 250 * 125 = 500ms
	sei();
}

float filtreFenetre(float tempRaw)
{
	float valFiltre = 0;
tblData[tblIndex++] = tempRaw;
if (tblIndex >= FILTRE_SIZE)
{
	tblFlag = 1;
	tblIndex = 0;
}
if (tblFlag)
{
	for (uint8_t i = 0; i < FILTRE_SIZE; i++)
	{
		valFiltre += tblData[i];
	}
	valFiltre /= FILTRE_SIZE;
}
    else
    {
        valFiltre = tempRaw;
    }
    return valFiltre;
}
