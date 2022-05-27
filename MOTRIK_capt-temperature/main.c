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
#define FILTRE_ECART_MAX 5

#define DEL_B_INIT()	(DDRB |= (1<<4)) //initialise PB4 comme étant une sortie. (del bleue)
#define DEL_J_INIT()	(DDRB |= (1<<5)) //initialise PB5 comme étant une sortie. (del jaune)
#define DEL_R_INIT()	(DDRB |= (1<<7)) //initialise PB7 comme étant une sortie. (del rouge)
#define DEL_B_SET(a)	(PORTB = (PORTB & ~(1<<4)) | ((a && 1) << 4)) //État de la del bleue.
#define DEL_J_SET(a)	(PORTB = (PORTB & ~(1<<5)) | ((a && 1) << 5)) //État de la del jaune.
#define DEL_R_SET(a)	(PORTB = (PORTB & ~(1<<7)) | ((a && 1) << 7)) //État de la del rouge.

#define CAPT_1	0
#define CAPT_2	1
#define CAPT_3	2

/************
 * VARIABLE *
 ************/
volatile uint8_t cntDixMs = 0;
volatile uint8_t dixMSFlag = 0;
uint8_t msgTemp[4] = {'<',1,0,'>'};
uint8_t msgTemp2[4] = {'<',2,0,'>'};
uint8_t msgTemp3[4] = {'<',3,0,'>'};
uint16_t adcValTemp = 0;
uint16_t adcValTemp2 = 0;
uint16_t adcValTemp3 = 0;
float temperatureAcquisition = 0;
float temperatureFiltered = 0;
float temperatureAcquisition2 = 0;
float temperatureFiltered2 = 0;
float temperatureAcquisition3 = 0;
float temperatureFiltered3 = 0;
uint8_t tblFlag = 0;
uint8_t tblFlag2 = 0;
uint8_t tblFlag3 = 0;
float tblData[FILTRE_SIZE];
float tblData2[FILTRE_SIZE];
float tblData3[FILTRE_SIZE];
uint8_t tblIndex = 0;
uint8_t tblIndex2 = 0;
uint8_t tblIndex3 = 0;
float valFiltre = 0;
float valFiltre2 = 0;
float valFiltre3 = 0;


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
float filtreFenetre(float tempRaw, uint8_t temp);


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
			adcValTemp = adcGetValue(CAPT_1); // Lecture du canal 1 du ADC.
			temperatureFiltered = filtreFenetre(temperatureAcquisition,CAPT_1);
			//sprintf(msgTemp, "Temperature-#%d: %d %0.1f \n\r", (CAPT_1+1), adcValTemp, temperatureAcquisition/*, temperatureFiltered*/); // Conversion de la mesure de température en string.
			//usartSendString(msgTemp);
			msgTemp[2] = (uint8_t)temperatureFiltered;
			usartSendBytes(msgTemp,4);
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//adcValTemp2 = adcGetValue(CAPT_2); // Lecture du canal 1 du ADC.
			//temperatureFiltered2 = filtreFenetre(temperatureAcquisition2,CAPT_2);
			////sprintf(msgTemp2, "Temperature-#%d: %d %0.1f \n\r", (CAPT_2+1), adcValTemp2, temperatureAcquisition2/*, temperatureFiltered2*/); // Conversion de la mesure de température en string.
			////usartSendString(msgTemp2);
			//msgTemp2[2] = (uint8_t)temperatureAcquisition2;
			//usartSendBytes(msgTemp2,4);
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//adcValTemp3 = adcGetValue(CAPT_3); // Lecture du canal 1 du ADC.
			//temperatureFiltered3 = filtreFenetre(temperatureAcquisition3,CAPT_3);
			////sprintf(msgTemp3, "Temperature-#%d: %d %0.1f \n\r", (CAPT_3+1), adcValTemp3, temperatureAcquisition3/*, temperatureFiltered*/); // Conversion de la mesure de température en string.
			////usartSendString(msgTemp3);
			//msgTemp3[2] = (uint8_t)temperatureAcquisition3;
			//usartSendBytes(msgTemp3,4);
		}
		temperatureAcquisition = ((1023.00-((float)adcValTemp ))-189.64)/4; // Puisque le capteur de température est un capteur linéaire, 22 à été obtenu en prenant une valeur en début et en la divisant par un nombre qui nous permet d'arriver à la température actuelle.
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//temperatureAcquisition2 = ((1023.00-((float)adcValTemp2 )));
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//temperatureAcquisition3 = ((1023.00-((float)adcValTemp3 )));
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
	if (cntDixMs >= 10)
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
	DEL_B_INIT();
	DEL_J_INIT();
	DEL_R_INIT();
	DEL_B_SET(1);
	DEL_J_SET(1);
	DEL_R_SET(1);
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

float filtreFenetre(float tempRaw, uint8_t temp)
{
	float returnValue = 0;
	switch(temp)
	{
	case 0:
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
		returnValue = valFiltre;
		break;
	case 1:
		if (tblIndex2 >= FILTRE_SIZE)
		{
			tblFlag2 = 1;
			tblIndex2 = 0;
		}
		if (!tblFlag2)
		{
			tblData2[tblIndex2++] = tempRaw;
		}
		if (tblFlag2)
		{

			for (uint8_t i = 0; i < FILTRE_SIZE; i++)
			{
				valFiltre2 += tblData2[i];
			}
			valFiltre2 /= FILTRE_SIZE;
			if (((tempRaw - valFiltre2) < FILTRE_ECART_MAX) && ((valFiltre2 - tempRaw) < FILTRE_ECART_MAX))
			{
				tblData2[tblIndex2++] = tempRaw;
			}
		}
		else
		{
			valFiltre2 = 0;
		}
		returnValue = valFiltre2;
		break;
	case 2:
		if (tblIndex3 >= FILTRE_SIZE)
		{
			tblFlag3 = 1;
			tblIndex3 = 0;
		}
		if (!tblFlag3)
		{
			tblData3[tblIndex3++] = tempRaw;
		}
		if (tblFlag3)
		{

			for (uint8_t i = 0; i < FILTRE_SIZE; i++)
			{
				valFiltre3 += tblData3[i];
			}
			valFiltre3 /= FILTRE_SIZE;
			if (((tempRaw - valFiltre3) < FILTRE_ECART_MAX) && ((valFiltre3 - tempRaw) < FILTRE_ECART_MAX))
			{
				tblData3[tblIndex3++] = tempRaw;
			}
		}
		else
		{
			valFiltre3 = 0;
		}
		returnValue = valFiltre3;
		break;
	}
	return returnValue;
}
