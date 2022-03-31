/*
Module qui regroupe les fonctions de la minuterie programmable

Auteur:
David Favreau

Date: 25/10/2021

Fonctions disponibles:
saisie_parametres
*/

/*Librairires et modules*/
#include "minuterie.h"

/*Constantes*/
#define DEMANDE_JOUR 0
#define JOUR_DEPART 1
#define JOUR_FIN 2
#define JOUR_ON 3
#define JOUR_OFF 4
#define DEMANDE_NUIT 5
#define NUIT_DEPART 6
#define NUIT_FIN 7
#define NUIT_ON 8
#define NUIT_OFF 9
#define CONFIG 10
#define CONFIG_FIN 11
#define SEC_30 30
#define MAX_HEURE 23
#define MAX_MINUTE 30

#define WRITE_CONTROL_REG 0x8F
#define READ_CONTROL_REG 0x0F
#define WRITE_TIME_REG 0x80
#define READ_TIME_REG 0x00
#define PROG_TEMPS_FIN 3
#define PROG_SECONDES 2
#define PROG_MINUTES 1
#define PROG_HEURES 0
#define MAX_HEURES 23
#define MAX_MINUTES 59
#define MAX_SECONDES 59
/*Structs*/


/*Parametres librairies*/

/*Definitions des fonctions privées*/
static uint8_t convertir_octet_input(uint8_t value){
  uint8_t convertedVal = value - 6 * (value >> 4) ;
  return convertedVal ;
}

static uint8_t convertir_octet_output(uint8_t value){
  uint8_t convertedVal = value +6 * (value / 10) ;
  return convertedVal ;
}

/*Definitions des fonctions publiques*/
void init_boutons(t_boutons* etats)
{
	//On initialise les boutons a zero
    etats->plus_etat_actuel = LOW;
    etats->plus_etat_precedent = LOW;
    etats->moins_etat_actuel = LOW;
    etats->moins_etat_precedent = LOW;
    etats->enter_etat_actuel = LOW;
    etats->enter_etat_precedent = LOW;
    etats->back_etat_actuel = LOW;
    etats->back_etat_precedent = LOW;
}

void rtc_init(uint8_t chip_select, t_rtc* temps)
{
	temps->secondes = 0;
	temps->minutes = 0;
	temps->heures = 0;
	temps->prog_fini = 0;
	
	digitalWrite(chip_select, HIGH);
	SPI.transfer(WRITE_CONTROL_REG);
	SPI.transfer(0b00000000);
	digitalWrite(chip_select, LOW);
	delay(10);
}

void rtc_ecrire_temps(uint8_t chip_select, t_rtc* temps)
{
	digitalWrite(chip_select, HIGH) ;
	SPI.transfer(WRITE_TIME_REG) ;
	SPI.transfer(convertir_octet_output(temps->secondes)) ;
	SPI.transfer(convertir_octet_output(temps->minutes)) ;
	SPI.transfer(convertir_octet_output(temps->heures)) ;
	digitalWrite(chip_select, LOW) ;
	delay(10) ;
}

void rtc_lire_temps(uint8_t chip_select, t_rtc* temps)
{
	digitalWrite(chip_select, HIGH) ;
	SPI.transfer(READ_TIME_REG) ;
	temps->secondes = convertir_octet_input(SPI.transfer(READ_TIME_REG)) ;
	temps->minutes = convertir_octet_input(SPI.transfer(READ_TIME_REG)) ;
	temps->heures = convertir_octet_input(SPI.transfer(READ_TIME_REG));
	digitalWrite(chip_select, LOW) ;
	delay(10) ;
	}

void init_cycle(t_temps* cycle)
{
	cycle->heure_depart = 0;
	cycle->heure_fin = 0;
	cycle->minute_on = 0;
	cycle->seconde_on = 0;
	cycle->minute_off = 0;
	cycle->seconde_off = 0;
	cycle->prog_fini = 0;
}

void saisie_temps(t_boutons* etats, uint8_t chip_select, t_rtc* temps, LiquidCrystal_I2C lcd)
{
	uint8_t etape_config = PROG_HEURES;
	
	init_boutons(etats);
	
	//Affichage initial
	lcd.clear();
    lcd.print("Mode programmation  ");
	lcd.setCursor(0,1);
	lcd.print("Real-Time Clock     ");
    lcd.setCursor(0, 3);
    lcd.write(0);
    lcd.print("+ ");
    lcd.write(1);
    lcd.print("- ");
    lcd.write(2);
    lcd.print("BACK ");
    lcd.write(3);
    lcd.print("ENTER  ");
	
	while(etape_config != PROG_TEMPS_FIN)
	{
		switch(etape_config)
		{
			case PROG_HEURES:
				lcd.setCursor(0,2);
				lcd.print("                    ");
				lcd.setCursor(0,2);
				lcd.print("Heure(24): ");
				lcd.print(temps->heures);
				lcd.print("h");
				
				while (etape_config == PROG_HEURES)
				{
					//On lit l'etat actuel des boutons    
					etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
					etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
					etats->enter_etat_actuel = digitalRead(ENTER);
					etats->back_etat_actuel = digitalRead(BACK);              
					
					if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
					{
						if (etats->plus_etat_actuel == HIGH && temps->heures < MAX_HEURES)
						{
							temps->heures += 1;
						}
						else if (etats->moins_etat_actuel && temps->heures > 0)
						{
							temps->heures -= 1;
						}
						lcd.setCursor(11, 2);
						lcd.print("   ");
						lcd.setCursor(11, 2);
						lcd.print(temps->heures);
						lcd.print("h");

						if (etats->enter_etat_actuel == HIGH)
						{
						  etape_config = PROG_MINUTES;
						}
						
						
						delay(50);   
					 }
						etats->plus_etat_precedent = etats->plus_etat_actuel;
						etats->moins_etat_precedent = etats->moins_etat_actuel;
						etats->enter_etat_precedent = etats->enter_etat_actuel;
						etats->back_etat_precedent = etats->back_etat_actuel;
				}
            break;
			
			case PROG_MINUTES:
				lcd.setCursor(0,2);
				lcd.print("                    ");
				lcd.setCursor(0,2);
				lcd.print("Minutes: ");
				lcd.print(temps->minutes);
				lcd.print("min");
				
				while (etape_config == PROG_MINUTES)
				{
					//On lit l'etat actuel des boutons    
					etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
					etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
					etats->enter_etat_actuel = digitalRead(ENTER);
					etats->back_etat_actuel = digitalRead(BACK);              
					
					if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
					{
						if (etats->plus_etat_actuel == HIGH && temps->minutes < MAX_MINUTES)
						{
							temps->minutes += 1;
						}
						else if (etats->moins_etat_actuel && temps->minutes > 0)
						{
							temps->minutes -= 1;
						}
						
						lcd.setCursor(9, 2);
						lcd.print("     ");
						lcd.setCursor(9, 2);
						lcd.print(temps->minutes);
						lcd.print("min");

						if (etats->enter_etat_actuel == HIGH)
						{
						  etape_config = PROG_SECONDES;
						}
						else if(etats->back_etat_actuel == HIGH)
						{
							etape_config = PROG_HEURES;
						}							
						
						
						delay(50);   
					 }
						etats->plus_etat_precedent = etats->plus_etat_actuel;
						etats->moins_etat_precedent = etats->moins_etat_actuel;
						etats->enter_etat_precedent = etats->enter_etat_actuel;
						etats->back_etat_precedent = etats->back_etat_actuel;
				}
            break;
			
			default:
				lcd.setCursor(0,2);
				lcd.print("                    ");
				lcd.setCursor(0,2);
				lcd.print("Secondes: ");
				lcd.print(temps->secondes);
				lcd.print("sec");
				
				while (etape_config == PROG_SECONDES)
				{
					//On lit l'etat actuel des boutons    
					etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
					etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
					etats->enter_etat_actuel = digitalRead(ENTER);
					etats->back_etat_actuel = digitalRead(BACK);              
					
					if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
					{
						if (etats->plus_etat_actuel == HIGH && temps->secondes < MAX_SECONDES)
						{
							temps->secondes += 1;
						}
						else if (etats->moins_etat_actuel && temps->secondes > 0)
						{
							temps->secondes -= 1;
						}
						
						lcd.setCursor(10, 2);
						lcd.print("     ");
						lcd.setCursor(10, 2);
						lcd.print(temps->secondes);
						lcd.print("sec");

						if (etats->enter_etat_actuel == HIGH)
						{
							temps->prog_fini = 1;
							etape_config = PROG_TEMPS_FIN;
						}
						else if(etats->back_etat_actuel == HIGH)
						{
							etape_config = PROG_MINUTES;
						}							
						
						
						delay(50);   
					 }
						etats->plus_etat_precedent = etats->plus_etat_actuel;
						etats->moins_etat_precedent = etats->moins_etat_actuel;
						etats->enter_etat_precedent = etats->enter_etat_actuel;
						etats->back_etat_precedent = etats->back_etat_actuel;
				}
            break;
			
						
			
		}
	}
	
	
}
void saisie_parametres(t_boutons* etats, t_temps* jour, t_temps* nuit, LiquidCrystal_I2C lcd)
{
    uint8_t etape_config = DEMANDE_JOUR; //Indice pour savoir si la configuration est terminée
    
    //On initialise les boutons a zero
    init_boutons(etats);
	  
    //Affichage initial
	lcd.clear();
    lcd.print("Mode programmation  ");; 
    lcd.setCursor(0, 3);
    lcd.write(0);
    lcd.print("+ ");
    lcd.write(1);
    lcd.print("- ");
    lcd.write(2);
    lcd.print("BACK ");
    lcd.write(3);
    lcd.print("ENTER  ");

    //Boucle pour programmer la minuterie
    while (etape_config != CONFIG_FIN)
    {   
        switch (etape_config)
        {
		case DEMANDE_JOUR:
			lcd.setCursor(0, 1);
            lcd.print("Programmer JOUR?    ");
            lcd.setCursor(0, 2);
            lcd.print("OUI(ENTER) NON(BACK)");
            lcd.setCursor(10, 2);
            while(etape_config == DEMANDE_JOUR)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);
              
            if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
            {
                if (etats->enter_etat_actuel == HIGH)
                {
                    etape_config = JOUR_DEPART;
					jour->prog_fini = 1;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = DEMANDE_NUIT;
					jour->prog_fini = 0;
                }
				delay(50);
            }
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
			break;
			
        case JOUR_DEPART:
            //Affichage pour programmer l'heure de depart
            lcd.setCursor(0, 1);
            lcd.print("Jour heure de DEPART");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Heure(24)  %.2d:00:00 ", jour->heure_depart);
			lcd.setCursor(0, 2);
            lcd.print("Heure(24): ");
			lcd.print(jour->heure_depart);
			lcd.print("h");

            while(etape_config == JOUR_DEPART)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);              
              if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
              {
                if (etats->plus_etat_actuel == HIGH && jour->heure_depart < MAX_HEURE)
                {
                 jour->heure_depart += 1;
                }
                else if (etats->moins_etat_actuel && jour->heure_depart > 0)
                {
                  jour->heure_depart -= 1;
                }
                lcd.setCursor(11, 2);
				lcd.print("   ");
				lcd.setCursor(11, 2);
                //sprintf(ecrire_temps, "%.2d", jour->heure_depart);
                lcd.print(jour->heure_depart);
                lcd.print("h");

                if (etats->enter_etat_actuel == HIGH)
                {
                  etape_config = JOUR_FIN;
                }
				else if (etats->back_etat_actuel == HIGH)
                {
                  etape_config = DEMANDE_JOUR;
                }
				delay(50);   
              }
				etats->plus_etat_precedent = etats->plus_etat_actuel;
				etats->moins_etat_precedent = etats->moins_etat_actuel;
				etats->enter_etat_precedent = etats->enter_etat_actuel;
				etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;

        case JOUR_FIN:
            //Affichage pour programmer l'heure de fin
            lcd.setCursor(0, 1);
            lcd.print("Jour heure de FIN   ");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Heure(24)  %.2d:00:00 ", jour->heure_fin);
			lcd.setCursor(0, 2);
			lcd.print("Heure(24): ");
            lcd.print(jour->heure_fin);
			lcd.print("h");
            

            while(etape_config == JOUR_FIN)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);  
              if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
              {
                if (etats->plus_etat_actuel == HIGH && jour->heure_fin < MAX_HEURE)
                {
                  jour->heure_fin += 1;
                }
                else if (etats->moins_etat_actuel && jour->heure_fin > 0)
                {
                  jour->heure_fin -= 1;
                }
				lcd.setCursor(11, 2);
				lcd.print("   ");
				//sprintf(ecrire_temps, "%.2d", jour->heure_fin);
				lcd.setCursor(11, 2);
                lcd.print(jour->heure_fin);
				lcd.print("h");
            
                if (etats->enter_etat_actuel == HIGH)
                {
                  etape_config = JOUR_ON;
                }
                else if(etats->back_etat_actuel == HIGH)
                {
                  etape_config = JOUR_DEPART;
                }
                delay(50);
              }
              etats->plus_etat_precedent = etats->plus_etat_actuel;
              etats->moins_etat_precedent = etats->moins_etat_actuel;
              etats->enter_etat_precedent = etats->enter_etat_actuel;
              etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;

        case JOUR_ON:
            //Affichage pour programmer l'interval allumer
            lcd.setCursor(0, 1);
            lcd.print("Jour interval ON    ");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Temps ON  %.2d:%.2d  ", jour->minute_on, jour->seconde_on);
			lcd.setCursor(0, 2);
            lcd.print("Temps ON: ");
			if(jour->minute_on > 9)
			{
				lcd.setCursor(10,2);
			}
			else
			{
				lcd.setCursor(11,2);
			}
			lcd.print(jour->minute_on);
			lcd.print(":");
			lcd.print(jour->seconde_on);
			lcd.setCursor(14,2);
			lcd.print("0");

            while(etape_config == JOUR_ON)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);  

              if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
              {
                if (etats->plus_etat_actuel == HIGH && jour->minute_on < MAX_MINUTE)
                {
                    if (jour->seconde_on == SEC_30)
                    {
                        jour->seconde_on = 0;
                        jour->minute_on += 1;
                    }
                    else
                    {
                        jour->seconde_on = SEC_30;
                    }
                }
                else if (etats->moins_etat_actuel && (jour->minute_on > 0 || jour->seconde_on == SEC_30))
                {
                    if (jour->seconde_on == SEC_30)
                    {
                        jour->seconde_on = 0;
                    }
                    else
                    {
                        jour->seconde_on = SEC_30;
                        jour->minute_on -= 1;
                    }
                }
				lcd.setCursor(10,2);
				lcd.print("    ");
                //sprintf(ecrire_temps, "%.2d:%.2d      ", jour->minute_on, jour->seconde_on);
				if(jour->minute_on > 9)
				{
					lcd.setCursor(10,2);
				}
				else
				{
					lcd.setCursor(11,2);
				}
                lcd.print(jour->minute_on);
				lcd.print(":");
				lcd.print(jour->seconde_on);
				lcd.setCursor(15,2);

                if (etats->enter_etat_actuel == HIGH)
                {
                    etape_config = JOUR_OFF;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = JOUR_FIN;
                }
				delay(50);
              }
              etats->plus_etat_precedent = etats->plus_etat_actuel;
              etats->moins_etat_precedent = etats->moins_etat_actuel;
              etats->enter_etat_precedent = etats->enter_etat_actuel;
              etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;

        case JOUR_OFF:
            //Affichage pour programmer l'interval éteint
            lcd.setCursor(0, 1);
            lcd.print("Jour interval OFF   ");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Temps OFF  %.2d:%.2d ", jour->minute_off, jour->seconde_off);
			lcd.setCursor(0, 2);
            lcd.print("Temps OFF: ");
			if(jour->minute_off > 9)
			{
				lcd.setCursor(11,2);
			}
			else
			{
				lcd.setCursor(12,2);
			}
			lcd.print(jour->minute_off);
			lcd.print(":");
			lcd.print(jour->seconde_off);
			lcd.setCursor(15, 2);
			lcd.print("0");
			
            while(etape_config == JOUR_OFF)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);  
            if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
            {
                if (etats->plus_etat_actuel == HIGH && jour->minute_off < MAX_MINUTE)
                {
                    if (jour->seconde_off == SEC_30)
                    {
                        jour->seconde_off = 0;
                        jour->minute_off += 1;
                    }
                    else
                    {
                        jour->seconde_off = SEC_30;
                    }
                }
                else if (etats->moins_etat_actuel && (jour->minute_off > 0 || jour->seconde_off == SEC_30))
                {
                    if (jour->seconde_off == SEC_30)
                    {
                        jour->seconde_off = 0;
                    }
                    else
                    {
                        jour->seconde_off = SEC_30;
                        jour->minute_off -= 1;
                    }
                }
				
				lcd.setCursor(11, 2);
				lcd.print("    ");
                //sprintf(ecrire_temps, "%.2d:%.2d     ", jour->minute_off, jour->seconde_off);
				if(jour->minute_off > 9)
				{
					lcd.setCursor(11,2);
				}
				else
				{
					lcd.setCursor(12,2);
				}
                lcd.print(jour->minute_off);
				lcd.print(":");
				lcd.print(jour->seconde_off);
                lcd.setCursor(16, 2);
				
                if (etats->enter_etat_actuel == HIGH)
                {
                    etape_config = DEMANDE_NUIT;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = JOUR_ON;
                }
				delay(50);
            }
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;
			
		case DEMANDE_NUIT:
			lcd.setCursor(0, 1);
            lcd.print("Programmer NUIT?    ");
            lcd.setCursor(0, 2);
            lcd.print("OUI(ENTER) NON(BACK)");
            lcd.setCursor(10, 2);
            while(etape_config == DEMANDE_NUIT)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);              
            if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
            {
                if (etats->enter_etat_actuel == HIGH)
                {
                    etape_config = NUIT_DEPART;
					nuit->prog_fini = 1;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = CONFIG;
					nuit->prog_fini = 0;
                }
				delay(50);
            }
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
			break;
			
        case NUIT_DEPART:
            //Affichage pour programmer l'heure de depart
            lcd.setCursor(0, 1);
            lcd.print("Nuit heure de DEPART");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Heure(24)  %.2d:00:00 ", nuit->heure_depart);
            lcd.setCursor(0, 2);
            lcd.print("Heure(24): ");
			lcd.print(nuit->heure_depart);
			lcd.print("h");

            while(etape_config == NUIT_DEPART)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);  
				if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
				{
					if (etats->plus_etat_actuel == HIGH && nuit->heure_depart < MAX_HEURE)
					{
						nuit->heure_depart += 1;
					}
					else if (etats->moins_etat_actuel && nuit->heure_depart > 0)
					{
						nuit->heure_depart -= 1;
					}
					
					lcd.setCursor(11, 2);
					lcd.print("   ");
					lcd.setCursor(11, 2);
					//sprintf(ecrire_temps, "%.2d", nuit->heure_depart);
					lcd.print(nuit->heure_depart);
					lcd.print("h");
					
					if (etats->enter_etat_actuel == HIGH)
					{
						etape_config = NUIT_FIN;
					}
					else if (etats->back_etat_actuel == HIGH)
					{
						etape_config = DEMANDE_NUIT;
					}
					delay(50);
				}
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;

        case NUIT_FIN:
            //Affichage pour programmer l'heure de fin
            lcd.setCursor(0, 1);
            lcd.print("Nuit heure de FIN   ");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Heure(24)  %.2d:00:00 ", nuit->heure_fin);
			lcd.setCursor(0, 2);
			lcd.print("Heure(24): ");
            lcd.print(nuit->heure_fin);
			lcd.print("h");

            while(etape_config == NUIT_FIN)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);  
            if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
            {
                if (etats->plus_etat_actuel == HIGH && nuit->heure_fin < MAX_HEURE)
                {
                    nuit->heure_fin += 1;
                }
                else if (etats->moins_etat_actuel && nuit->heure_fin > 0)
                {
                    nuit->heure_fin -= 1;
                }
				
				lcd.setCursor(11, 2);
				lcd.print("   ");
				//sprintf(ecrire_temps, "%.2d", nuit->heure_fin);
				lcd.setCursor(11, 2);
                lcd.print(nuit->heure_fin);
				lcd.print("h");
				
                if (etats->enter_etat_actuel == HIGH)
                {
                    etape_config = NUIT_ON;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = NUIT_DEPART;
                }
				delay(50);
            }
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;

        case NUIT_ON:
            //Affichage pour programmer l'interval allumer
            lcd.setCursor(0, 1);
            lcd.print("Nuit interval ON    ");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Temps ON  %.2d:%.2d", nuit->minute_on, nuit->seconde_on);
            lcd.setCursor(0, 2);
            lcd.print("Temps ON: ");
			if(nuit->minute_on > 9)
			{
				lcd.setCursor(10,2);
			}
			else
			{
				lcd.setCursor(11,2);
			}
			lcd.print(nuit->minute_on);
			lcd.print(":");
			lcd.print(nuit->seconde_on);
			lcd.setCursor(14,2);
			lcd.print("0");

            while(etape_config == NUIT_ON)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);  
            if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
            {
                if (etats->plus_etat_actuel == HIGH && nuit->minute_on < MAX_MINUTE)
                {
                    if (nuit->seconde_on == SEC_30)
                    {
                        nuit->seconde_on = 0;
                        nuit->minute_on += 1;
                    }
                    else
                    {
                        nuit->seconde_on = SEC_30;
                    }
                }
                else if (etats->moins_etat_actuel && (nuit->minute_on > 0 || nuit->seconde_on == SEC_30))
                {
                    if (nuit->seconde_on == SEC_30)
                    {
                        nuit->seconde_on = 0;
                    }
                    else
                    {
                        nuit->seconde_on = SEC_30;
                        nuit->minute_on -= 1;
                    }
                }
				
                lcd.setCursor(10,2);
				lcd.print("    ");
                //sprintf(ecrire_temps, "%.2d:%.2d      ", nuit->minute_on, nuit->seconde_on);
				if(nuit->minute_on > 9)
				{
					lcd.setCursor(10,2);
				}
				else
				{
					lcd.setCursor(11,2);
				}
                lcd.print(nuit->minute_on);
				lcd.print(":");
				lcd.print(nuit->seconde_on);
				lcd.setCursor(15,2);
				
                if (etats->enter_etat_actuel == HIGH)
                {
                    etape_config = NUIT_OFF;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = NUIT_FIN;
                }
				delay(50);
            }
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;

        case NUIT_OFF:
            //Affichage pour programmer l'interval éteint
            lcd.setCursor(0, 1);
            lcd.print("Nuit interval OFF   ");
            lcd.setCursor(0, 2);
			lcd.print("                    ");
            //sprintf(ecrire_temps, "Temps OFF  %.2d:%.2d", nuit->minute_off, nuit->seconde_off);
            lcd.setCursor(0, 2);
            lcd.print("Temps OFF: ");
			if(nuit->minute_off > 9)
			{
				lcd.setCursor(11,2);
			}
			else
			{
				lcd.setCursor(12,2);
			}
			lcd.print(nuit->minute_off);
			lcd.print(":");
			lcd.print(nuit->seconde_off);
			lcd.setCursor(15, 2);
			lcd.print("0");

            while(etape_config == NUIT_OFF)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);  
            if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
            {
                if (etats->plus_etat_actuel == HIGH && nuit->minute_off < MAX_MINUTE)
                {
                    if (nuit->seconde_off == SEC_30)
                    {
                        nuit->seconde_off = 0;
                        nuit->minute_off += 1;
                    }
                    else
                    {
                        nuit->seconde_off = SEC_30;
                    }
                }
                else if (etats->moins_etat_actuel && (nuit->minute_off > 0 || nuit->seconde_off == SEC_30))
                {
                    if (nuit->seconde_off == SEC_30)
                    {
                        nuit->seconde_off = 0;
                    }
                    else
                    {
                        nuit->seconde_off = SEC_30;
                        nuit->minute_off -= 1;
                    }
                }
				
                lcd.setCursor(11, 2);
				lcd.print("    ");
                //sprintf(ecrire_temps, "%.2d:%.2d     ", nuit->minute_off, nuit->seconde_off);
				if(nuit->minute_off > 9)
				{
					lcd.setCursor(11,2);
				}
				else
				{
					lcd.setCursor(12,2);
				}
				
                lcd.print(nuit->minute_off);
				lcd.print(":");
				lcd.print(nuit->seconde_off);
                lcd.setCursor(16, 2);
				
                if (etats->enter_etat_actuel == HIGH)
                {
                    etape_config = CONFIG;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = NUIT_ON;
                }
				delay(50);
            }
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
            break;
        //Case CONFIG => Programmation terminée?
        default:
            lcd.setCursor(0, 1);
            lcd.print("Fini de programmer?");
            lcd.setCursor(0, 2);
            lcd.print("OUI(ENTER) NON(BACK)");
            lcd.setCursor(10, 2);
            while(etape_config == CONFIG)
            {
              //On lit l'etat actuel des boutons    
              etats->plus_etat_actuel = digitalRead(BOUTON_PLUS);
              etats->moins_etat_actuel = digitalRead(BOUTON_MOINS);
              etats->enter_etat_actuel = digitalRead(ENTER);
              etats->back_etat_actuel = digitalRead(BACK);              
            if ((etats->plus_etat_actuel != etats->plus_etat_precedent) || (etats->moins_etat_actuel != etats->moins_etat_precedent) || (etats->enter_etat_actuel != etats->enter_etat_precedent) || (etats->back_etat_actuel != etats->back_etat_precedent))
            {
                if (etats->enter_etat_actuel == HIGH)
                {
					
                    etape_config = CONFIG_FIN;
                }
                else if (etats->back_etat_actuel == HIGH)
                {
                    etape_config = DEMANDE_JOUR;
                }
            }
			delay(50);
            etats->plus_etat_precedent = etats->plus_etat_actuel;
            etats->moins_etat_precedent = etats->moins_etat_actuel;
            etats->enter_etat_precedent = etats->enter_etat_actuel;
            etats->back_etat_precedent = etats->back_etat_actuel;
            }
        }
        
    }
}