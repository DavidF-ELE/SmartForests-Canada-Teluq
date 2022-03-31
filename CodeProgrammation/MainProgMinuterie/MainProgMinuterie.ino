/*
  Programme pour tester l'utilisation de boutons et affichage LCD
*/

/*Librairies et modules*/
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <minuterie.h>
#include <EEPROM.h>
#include <SPI.h>


/*Constantes*/
#define BOUTON_PLUS 6   //Digital pin #6 ou est connecter le bouton PLUS
#define BOUTON_MOINS 5  //Digital pin #5 ou est connecter le bouton MOINS
#define RELAIS_1 23          //Digital pin #23 ou est connecter un relais
#define ENTER 7         //Digital pin #7 ou est connecter le bouton ENTER
#define BACK 4          //Digital pin #4 ou est connecter le bouton BACK
#define CHIP_SELECT 17  //Digital pin #17 qui permet de choisir le peripherique SPI
#define ADDR_JOUR 0     //Addresse EEPROM ou se trouve le cycle de jour
#define JOUR_PROG 6     //Addresse EEPROM ou se trouve l'indice de programmation du cycle de jour
#define ADDR_NUIT 7     //Addresse EEPROM ou se trouve le cycle de nuit
#define NUIT_PROG 13    //Addresse EEPROM ou se trouve l'indice de programmation du cycle de nuit
#define TEMPS_PROG 14   //Adresse EEPROM ou se trouve l'indice de programmation de l'horloge

/*Structs*/

/*Prototypes*/


/*Variables Globales*/
t_boutons etats_boutons; //Variable pour enregistrer les etats des boutons
t_temps jour; //Variable contenant les parametres du cycle de jour
t_temps nuit; //Variable contenant les parametres du cycle de nuit
t_temps cycle_jour;
t_temps cycle_nuit;
t_rtc temps;
unsigned long avant;
unsigned long apres;

/*Parametres librairies*/
LiquidCrystal_I2C lcd(0x27,20,4);
/*Custom Characters*/
byte up_arrow[8] =
{
0b00000,
0b00100,
0b01110,
0b10101,
0b00100,
0b00100,
0b00100,
0b00000
};
byte down_arrow[8] =
{
0b00000,
0b00100,
0b00100,
0b00100,
0b10101,
0b01110,
0b00100,
0b00000
};
byte right_arrow[8] =
{
0b00000,
0b00100,
0b00110,
0b11111,
0b11111,
0b00110,
0b00100,
0b00000
};
byte left_arrow[8] =
{
0b00000,
0b00100,
0b01100,
0b11111,
0b11111,
0b01100,
0b00100,
0b00000
};

//Setup initial
void setup()
{
  Serial.begin(9600);
  SPI.begin() ;
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));

  lcd.init();                      //Initialisation de l'écran LCD
  lcd.backlight();                 //On active la backlight
  //Characteres fleches
  lcd.createChar(0, up_arrow);
  lcd.createChar(1, down_arrow);  
  lcd.createChar(2, left_arrow);
  lcd.createChar(3, right_arrow);

  //Les boutons sont des entrees
  pinMode(BOUTON_PLUS, INPUT);
  pinMode(BOUTON_MOINS, INPUT);
  pinMode(ENTER, INPUT);
  pinMode(BACK, INPUT);
  
  //La DEL est la sortie
  pinMode(RELAIS_1, OUTPUT);
  
  init_cycle(&jour);
  init_cycle(&nuit);
  rtc_init(CHIP_SELECT, &temps);
}

//Main
void loop()
{
  lcd.clear();

  if(EEPROM.read(JOUR_PROG) == 0 && EEPROM.read(NUIT_PROG) == 0)
  {
    init_cycle(&jour);
    init_cycle(&nuit);

    saisie_parametres(&etats_boutons, &jour,&nuit, lcd);
    
    EEPROM.write(ADDR_JOUR, jour.heure_depart);
    EEPROM.write(ADDR_JOUR+1, jour.heure_fin);
    EEPROM.write(ADDR_JOUR+2, jour.minute_on);
    EEPROM.write(ADDR_JOUR+3, jour.seconde_on);
    EEPROM.write(ADDR_JOUR+4, jour.minute_off);
    EEPROM.write(ADDR_JOUR+5, jour.seconde_off);
    EEPROM.write(ADDR_JOUR+6, jour.prog_fini);

    EEPROM.write(ADDR_NUIT, nuit.heure_depart);
    EEPROM.write(ADDR_NUIT+1, nuit.heure_fin);
    EEPROM.write(ADDR_NUIT+2, nuit.minute_on);
    EEPROM.write(ADDR_NUIT+3, nuit.seconde_on);
    EEPROM.write(ADDR_NUIT+4, nuit.minute_off);
    EEPROM.write(ADDR_NUIT+5, nuit.seconde_off);
    EEPROM.write(ADDR_NUIT+6, nuit.prog_fini);
  }
  else
  {
    jour.heure_depart = EEPROM.read(ADDR_JOUR);
    jour.heure_fin = EEPROM.read(ADDR_JOUR+1);
    jour.minute_on = EEPROM.read(ADDR_JOUR+2);
    jour.seconde_on = EEPROM.read(ADDR_JOUR+3);
    jour.minute_off = EEPROM.read(ADDR_JOUR+4);
    jour.seconde_off = EEPROM.read(ADDR_JOUR+5);
    jour.prog_fini = EEPROM.read(ADDR_JOUR+6);

    nuit.heure_depart = EEPROM.read(ADDR_NUIT);
    nuit.heure_fin = EEPROM.read(ADDR_NUIT+1);
    nuit.minute_on = EEPROM.read(ADDR_NUIT+2);
    nuit.seconde_on = EEPROM.read(ADDR_NUIT+3);
    nuit.minute_off = EEPROM.read(ADDR_NUIT+4);
    nuit.seconde_off = EEPROM.read(ADDR_NUIT+5);
    nuit.prog_fini = EEPROM.read(ADDR_NUIT+6);

  }

  if(EEPROM.read(TEMPS_PROG) == 0)
  {
    saisie_temps(&etats_boutons, CHIP_SELECT, &temps, lcd);
    rtc_ecrire_temps(CHIP_SELECT, &temps);
    EEPROM.write(TEMPS_PROG, temps.prog_fini); 
  }
  //Mise a jour de l'ecran
  lcd.clear();
  lcd.setCursor(0,3);
  lcd.print("Maintenir"); 
  lcd.write(2);
  lcd.print("BACK R.A.Z");

  //Les valeurs initials des cycles de jours et nuits
  cycle_jour = jour;
  cycle_nuit = nuit;
  init_boutons(&etats_boutons);
  
  while(etats_boutons.back_etat_actuel != HIGH)
  {
    avant = millis();
    rtc_lire_temps(CHIP_SELECT, &temps);
    lcd.setCursor(12,0);
    lcd.print("00:00:00");
    if(temps.heures > 9)
    {
      lcd.setCursor(12,0);
    }
    else
    {
      lcd.setCursor(13,0);
    }
    lcd.print(temps.heures);
    
    if(temps.minutes > 9)
    {
      lcd.setCursor(15,0);  
    }
    else
    {
      lcd.setCursor(16,0);        
    }
    lcd.print(temps.minutes);

    if(temps.secondes > 9)
    {
      lcd.setCursor(18,0);
    }
    else
    {
      lcd.setCursor(19,0);
    }
    lcd.print(temps.secondes);      

    

    if(temps.heures >= jour.heure_depart && temps.heures < jour.heure_fin && jour.prog_fini == 1)
    {
      cycle_nuit = nuit;

      lcd.setCursor(0,0);
      lcd.print("JOUR ");
      lcd.print(jour.heure_depart);
      lcd.write(3);
      lcd.print(jour.heure_fin);
      lcd.print("  ");
      lcd.setCursor(0,1);
      lcd.print("ON: 00:00");
      if(jour.minute_on > 9)
      {
        lcd.setCursor(4,1);  
      }
      else
      {
        lcd.setCursor(5,1);        
      }
      lcd.print(jour.minute_on);

      if(jour.seconde_on > 9)
      {
        lcd.setCursor(7,1);
      }
      else
      {
        lcd.setCursor(8,1);
      }
      lcd.print(jour.seconde_on);

      lcd.print(" OFF: 00:00");
      if(jour.minute_off > 9)
      {
        lcd.setCursor(15,1);  
      }
      else
      {
        lcd.setCursor(16,1);        
      }
      lcd.print(jour.minute_off);

      if(jour.seconde_off > 9)
      {
        lcd.setCursor(18,1);
      }
      else
      {
        lcd.setCursor(19,1);
      }
      lcd.print(jour.seconde_off);

      lcd.setCursor(0,2);

      if(cycle_jour.prog_fini == 1)
      {
        lcd.print("ON  en cours: 00:00");
        digitalWrite(RELAIS_1, HIGH);
        

        if(cycle_jour.minute_on > 9)
        {
          lcd.setCursor(14,2);          
        }
        else
        {
          lcd.setCursor(15,2);          
        }
        lcd.print(cycle_jour.minute_on);

        if(cycle_jour.seconde_on > 9)
        {
          lcd.setCursor(17,2);        
        }
        else
        {
          lcd.setCursor(18,2);
        }
        lcd.print(cycle_jour.seconde_on);

        if(cycle_jour.seconde_on == 0)
        {
          if(cycle_jour.minute_on == 0)
          {
            cycle_jour.prog_fini = 0;
            cycle_jour.minute_on = jour.minute_on;
            cycle_jour.seconde_on = jour.seconde_on;                    
          }
          else
          {
            cycle_jour.minute_on -= 1;
            cycle_jour.seconde_on = 59;
          }                           
        }
        else
        {
          cycle_jour.seconde_on -= 1;
        }       
      }
      else
      {
        lcd.print("OFF en cours: 00:00");
        digitalWrite(RELAIS_1, LOW);
        

        if(cycle_jour.minute_off > 9)
        {
          lcd.setCursor(14,2);          
        }
        else
        {
          lcd.setCursor(15,2);          
        }
        lcd.print(cycle_jour.minute_off);

        if(cycle_jour.seconde_off > 9)
        {
          lcd.setCursor(17,2);        
        }
        else
        {
          lcd.setCursor(18,2);
        }
        lcd.print(cycle_jour.seconde_off);

        if(cycle_jour.seconde_off == 0)
        {
          if(cycle_jour.minute_off == 0)
          {
            cycle_jour.prog_fini = 1;
            cycle_jour.minute_off = jour.minute_off;
            cycle_jour.seconde_off = jour.seconde_off;                    
          }
          else
          {
            cycle_jour.minute_off -= 1;
            cycle_jour.seconde_off = 59;
          }                           
        }
        else
        {
          cycle_jour.seconde_off -= 1;
        }   
        
      }
    }
    else if(((temps.heures >= nuit.heure_depart && temps.heures < nuit.heure_fin && nuit.heure_depart < nuit.heure_fin) || (((temps.heures >= nuit.heure_depart && temps.heures <= 23) || 
    (temps.heures >= 0 && temps.heures < nuit.heure_fin)) && nuit.heure_depart > nuit.heure_fin)) && nuit.prog_fini == 1)
    {
      cycle_jour = jour;
      lcd.setCursor(0,0);
      lcd.print("NUIT ");
      lcd.print(nuit.heure_depart);
      lcd.write(3);
      lcd.print(nuit.heure_fin);
      lcd.print("  ");
      lcd.setCursor(0,1);
      lcd.print("ON: 00:00");
      if(nuit.minute_on > 9)
      {
        lcd.setCursor(4,1);  
      }
      else
      {
        lcd.setCursor(5,1);        
      }
      lcd.print(nuit.minute_on);

      if(nuit.seconde_on > 9)
      {
        lcd.setCursor(7,1);
      }
      else
      {
        lcd.setCursor(8,1);
      }
      lcd.print(nuit.seconde_on);

      lcd.print(" OFF: 00:00");
      if(nuit.minute_off > 9)
      {
        lcd.setCursor(15,1);  
      }
      else
      {
        lcd.setCursor(16,1);        
      }
      lcd.print(nuit.minute_off);

      if(nuit.seconde_off > 9)
      {
        lcd.setCursor(18,1);
      }
      else
      {
        lcd.setCursor(19,1);
      }
      lcd.print(nuit.seconde_off);

      lcd.setCursor(0,2);

      if(cycle_nuit.prog_fini == 1)
      {
        lcd.print("ON  en cours: 00:00");
        digitalWrite(RELAIS_1, HIGH);
        

        if(cycle_nuit.minute_on > 9)
        {
          lcd.setCursor(14,2);          
        }
        else
        {
          lcd.setCursor(15,2);          
        }
        lcd.print(cycle_nuit.minute_on);

        if(cycle_nuit.seconde_on > 9)
        {
          lcd.setCursor(17,2);        
        }
        else
        {
          lcd.setCursor(18,2);
        }
        lcd.print(cycle_nuit.seconde_on);

        if(cycle_nuit.seconde_on == 0)
        {
          if(cycle_nuit.minute_on == 0)
          {
            cycle_nuit.prog_fini = 0;
            cycle_nuit.minute_on = nuit.minute_on;
            cycle_nuit.seconde_on = nuit.seconde_on;                    
          }
          else
          {
            cycle_nuit.minute_on -= 1;
            cycle_nuit.seconde_on = 59;
          }                           
        }
        else
        {
          cycle_nuit.seconde_on -= 1;
        }       
      }
      else
      {
        lcd.print("OFF en cours: 00:00");
        digitalWrite(RELAIS_1, LOW);
       
        if(cycle_nuit.minute_off > 9)
        {
          lcd.setCursor(14,2);          
        }
        else
        {
          lcd.setCursor(15,2);          
        }
        lcd.print(cycle_nuit.minute_off);

        if(cycle_nuit.seconde_off > 9)
        {
          lcd.setCursor(17,2);        
        }
        else
        {
          lcd.setCursor(18,2);
        }
        lcd.print(cycle_nuit.seconde_off);

        if(cycle_nuit.seconde_off == 0)
        {
          if(cycle_nuit.minute_off == 0)
          {
            cycle_nuit.prog_fini = 1;
            cycle_nuit.minute_off = nuit.minute_off;
            cycle_nuit.seconde_off = nuit.seconde_off;                    
          }
          else
          {
            cycle_nuit.minute_off -= 1;
            cycle_nuit.seconde_off = 59;
          }                           
        }
        else
        {
          cycle_nuit.seconde_off -= 1;
        }   
        
      }
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print("ATTENTE   ");     
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("JOUR: ");
      lcd.print(jour.heure_depart);
      lcd.write(3);
      lcd.print(jour.heure_fin);
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("NUIT: ");
      lcd.print(nuit.heure_depart);
      lcd.write(3);
      lcd.print(nuit.heure_fin);

      digitalWrite(RELAIS_1, LOW);
      

      cycle_jour = jour;
      cycle_nuit = nuit;
      
    }

    apres = millis();  
    delay(1000 - (apres - avant));
    etats_boutons.back_etat_actuel = digitalRead(BACK);
  }
  
  digitalWrite(RELAIS_1, LOW);
  lcd.clear();
  lcd.print("REMISE A ZERO");
  rtc_init(CHIP_SELECT, &temps);
  EEPROM.write(TEMPS_PROG, temps.prog_fini); 
  init_cycle(&jour);
  EEPROM.write(JOUR_PROG, jour.prog_fini);
  init_cycle(&nuit);
  EEPROM.write(NUIT_PROG, nuit.prog_fini);
  delay(3000);
    
}
