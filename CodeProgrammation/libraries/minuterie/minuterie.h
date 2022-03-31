#ifndef MINUTEIRE_H
#define MINUTERIE_H
/*
Module qui regroupe les fonctions de la minuterie programmable

Auteur:
David Favreau

Date: 25/10/2021

Fonctions disponibles:
saisie_parametres
*/

/*Librairires et modules*/
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SPI.h>


/*Constantes*/
#define BOUTON_PLUS 6 //Digital pin #10
#define BOUTON_MOINS 5 //Digital pin #2
#define LED 23          //Digital pin #3
#define ENTER 7        //Digital pin #5
#define BACK 4         //Digital pin #4

/*Structures*/
typedef struct t_boutons {
    uint8_t plus_etat_actuel; //Etat actuel du boutton PLUS
    uint8_t plus_etat_precedent; //Etat precedent du boutton PLUS
    uint8_t moins_etat_actuel;  //Etat actuel du boutton MOINS
    uint8_t moins_etat_precedent; //Etat precedent du boutton MOINS
    uint8_t enter_etat_actuel; //Etat actuel du boutton ENTER
    uint8_t enter_etat_precedent; //Etat precedent du boutton ENTER
    uint8_t back_etat_actuel;  //Etat actuel du boutton BACK
    uint8_t back_etat_precedent; //Etat precedent du boutton BACK
} t_boutons;

typedef struct t_temps {
    uint8_t heure_depart; 
    uint8_t heure_fin;
    uint8_t minute_on;
    uint8_t seconde_on;
    uint8_t minute_off;
    uint8_t seconde_off;
	uint8_t prog_fini;
}t_temps;

typedef struct t_rtc{
  uint8_t heures;
  uint8_t minutes;
  uint8_t secondes;
  uint8_t prog_fini;
}t_rtc;


#ifdef __cplusplus
extern "C" {
#endif

/*Fonctions publiques*/

void init_boutons(t_boutons* boutons);
void rtc_init(uint8_t chip_select, t_rtc* temps);
void rtc_ecrire_temps(uint8_t chip_select, t_rtc* temps);
void rtc_lire_temps(uint8_t chip_select, t_rtc* temps);
void saisie_temps(t_boutons* etats, uint8_t chip_select, t_rtc* temps, LiquidCrystal_I2C lcd);
void init_cycle(t_temps* cycle);
void saisie_parametres(t_boutons* etats, t_temps* jour, t_temps* nuit, LiquidCrystal_I2C lcd);

#ifdef __cplusplus
}
#endif

#endif

