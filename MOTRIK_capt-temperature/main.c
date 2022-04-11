/**
* @file		 main.c
* @brief	 Partie du projet Motrik qui sera utilisé afin de mesurer la température avec le LM65.
* @author	 Thomas Desrosiers
* @version	 2.1
* @date		 2022/02/11

* @mainpage  MOTRIK_capt-temperature
* @author 	 Thomas Desrosiers
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


/**********
 * DEFINE *
 **********/
#define FILTRE_SIZE 100
#define FILTRE_ECART_MAX 3


/************
 * VARIABLE *
 ************/
volatile uint8_t cntDixMs = 0;
volatile uint8_t dixMSFlag = 0;
char msgTemp[17];
uint16_t adcValTemp = 0;
float temperatureAcquisition = 0;
float temperatureFiltered = 0;
uint8_t tblFlag = 0;
float tblData[FILTRE_SIZE];
uint8_t tblIndex = 0;


/******************
 *      ENUM      *
 * STRUCT & UNION *
 ******************/


/**************************
 * PROTOTYPES DE FONCTION *
 **************************/
/**
 * @brief  Fonction d'initialisation du timer 0 avec une période de 1ms.
 */
void timer1Init();

/**
 *@brief  Fonction qui regroupe l'initialisation des différents I/O et des librairies.
 */
void miscInit(void);

/**
 * @brief  		   Fonction qui remplis un tableau de données et retourne la moyenne de ces valeurs.
 * @param tempRaw  Valeur brut mesurée et calculer depuis l'ADC.
 * @return 		   Moyenne calculée depuis le tableau.
 */
float filtreFenetre(float tempRaw);


/********
 * MAIN *
 ********/
int main(void)
{
	miscInit();
	while (1)
	{
		if (dixMSFlag) // Flag qui est vrai à chaque 10ms.
		{
			dixMSFlag = 0;
			adcValTemp = adcGetValue(0); // Lecture du canal 1 du ADC.
			temperatureFiltered = filtreFenetre(temperatureAcquisition);
			sprintf(msgTemp, "Temperature: %0.1f  %0.1f\n\r", temperatureAcquisition, temperatureFiltered); // Conversion de la mesure de température en string.
			usartSendString(msgTemp);
		}
		temperatureAcquisition = (float)adcValTemp / 8.903512712; // Puisque le capteur de température est un capteur linéaire, 22 à été obtenu en prenant une valeur en début et en la divisant par un nombre qui nous permet d'arriver à la température actuelle.
	}
}


/****************
 * INTERRUPTION *
 ****************/
/**
 *@brief  Le timer 1 est initialisé à 1ms. à chaques 1ms, cntDixFlag mesure est HAUT et après 10ms(10 x 1ms) cntDixFlag est HAUT.
 */
ISR(TIMER1_COMPA_vect)
{
	cntDixMs++;
	if (cntDixMs >= 100)
	{
		cntDixMs -= 10;
		dixMSFlag = 1; //À chaque 10ms. Ce flag sera utilisé pour changer les valeurs envoyés par les capteurs.
	}
}


/************************
 * DÉFINITION FONCTIONS *
 ************************/
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
	TCCR1B = (1 << WGM12);				 // mode CTC.
	TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler de 64.
	TIMSK1 |= (1 << OCIE1A);			 // Output Compare A Match Interrupt Enable.
	OCR1A = 250 - 1;					 // 62.5ns * 64 * 250 = 1ms.
	sei();
}

float filtreFenetre(float tempRaw)
{
	float valFiltre = 0;
	if (tblIndex >= FILTRE_SIZE)
	{
		tblFlag = 1;
		tblIndex = 0;
	}
	if (!tblFlag)
	{
		tblData[tblIndex++] = tempRaw;
	}
	if (tblFlag)
	{

		for (uint8_t i = 0; i < FILTRE_SIZE; i++)
		{
			valFiltre += tblData[i];
		}
		valFiltre /= FILTRE_SIZE;
		if (((tempRaw - valFiltre) < FILTRE_ECART_MAX) && ((valFiltre - tempRaw) < FILTRE_ECART_MAX))
		{
			tblData[tblIndex++] = tempRaw;
		}
	}
	else
	{
		valFiltre = 0;
	}
	return valFiltre;
}
