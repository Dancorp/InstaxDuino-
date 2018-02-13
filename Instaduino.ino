/* Projet Instaduino par Dancorp pour Polaroid-passion.com*/

/*------------------------------------------------------------------------*/
/*--------------------------------- INIT ---------------------------------*/
/*------------------------------------------------------------------------*/


#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include "IRremote.h"
int broche_reception = 7;
IRrecv irrecv(broche_reception);
decode_results decode_ir;
LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7); // 0x27 is the I2C bus address for an unmodified backpack

// Utilisation des E/S
int beeper = 8;
int shutter = 9;
int capteur = 10;
int ejmotor = 11;
int capot = 12;
int btnshutter = 13;
int shutalim = 5;
int shutin1 = 4;
int shutin2 = 3;
int ShutterState;

// caractères personalisés
byte bmexp[8] = {B11110, B10010, B11111, B11001, B11001, B01001, B01111,};
byte bsand[8] = {B11111, B10001, B01110, B00100, B01010, B11111, B11111,};
byte bflash[8] = {B00111, B01110, B01100, B00110, B01100, B11000, B10000,};

// variables pour la gestion des vitesses
// tableaux vitesses
int vms[20] = {2, 4, 8, 16, 33, 66, 125, 250, 500, 1000, 2000, 4000, 6000, 8000, 10000, 12000, 15000, 20000, 30000, 45000};
char txtvitesse[20][5] = {"500'", "250'", "125'", " 60'", " 30'", " 15'", "1/8'", "1/4'", "1/2'", "  1s", "  2s", "  4s", "  6s", "  8s", " 10s", " 12s", " 15s", " 20s", " 30s", " 45s"};
int pvitesse = 0;

// initialisation des txt variables du LCD
char ligne1[16] = "";
char ligne2[16] = "";

// initialisation options
boolean optmexpo = false; // option Multiexpo
boolean optauto = true; // Option Mode Auto
//boolean optpauset = false;
int optsand = 0; // Option pour le retardateur
int optflash = 0; // gestion futur du flash


/*------------------------------------------------------------------------*/
/*--------------------------------- SETUP --------------------------------*/
/*------------------------------------------------------------------------*/

void setup() {

  // activate LCD module
  lcd.begin (16, 2); // for 16 x 2 LCD module
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);
  Serial.begin(9600);
  irrecv.enableIRIn();
  irrecv.blink13(true);
  lcd.print("INSTADUINO! 0.2");
  lcd.setCursor (0, 0);


  // activation caractères perso sur les adresses mémoire
  lcd.createChar(0, bmexp);
  lcd.createChar(1, bsand);
  lcd.createChar(2, bflash);

  // activation ES
  pinMode(beeper, OUTPUT);
  pinMode(ejmotor, OUTPUT);
  pinMode(shutter, OUTPUT);
  pinMode(capteur, OUTPUT);
  pinMode(capot, OUTPUT);
  pinMode(btnshutter, INPUT);
  pinMode(shutalim, OUTPUT);
  pinMode(shutin1, OUTPUT);
  pinMode(shutin2, OUTPUT);




  // BEEP de Demarrage
  delay (2000);
  beep(2);

  // Lancement Ecran principal
  lcdupdate();

}


/*------------------------------------------------------------------------*/
/*---------------------------- FONCTIONS ---------------------------------*/
/*------------------------------------------------------------------------*/

// LCDUPDATE : MAJ Ecran principal

void lcdupdate() {
  lcd.clear();
  sprintf(ligne1, "[%s]", txtvitesse[pvitesse]);
  lcd.setCursor (0, 0);
  lcd.write(ligne1);
 
  if (optauto)  // Ajout du texte AUTO sur la 2nd ligne
  {
    lcd.setCursor (1, 0);
    lcd.print("AUTO");
  }

  if (optmexpo) // AJout du caractère perso Multi Expo
  {
    lcd.setCursor (6, 0);
    lcd.write(byte(0));
  }
  if (optsand != 0)  AJout des infos retardateur
  {
    lcd.setCursor (8, 0);
    lcd.write(byte(1));
    lcd.setCursor (9, 0);
    lcd.print(optsand);
  }
 
  if (optflash != 0) AJout du caractère perso Flash
  {
    lcd.setCursor (12, 0);
    lcd.write(byte(2));
    lcd.setCursor (13, 0);
    lcd.print(optflash);
  }

}


// Fonction DoubleExpo simple
void mexposhoot() {
  lcd.setCursor (0, 1);
  lcd.print("MultiExpo Shoot"); //On écrit le txt sur la 2nd ligne du LCD
  digitalWrite(ejmotor, HIGH);  // On coupe le moteur via le relais 5v
  delay (100);
  digitalWrite(shutter, HIGH); // On presse le bouton déclencheur photo
  delay (300);
  digitalWrite(shutter, LOW); // On le relache
  digitalWrite(capteur, HIGH); // On active le capteur d'éjection
  delay (500);
  digitalWrite(capteur, LOW);// On le relache, l'appareil pense avoir ejecté la photo
  delay (100);
  digitalWrite(ejmotor, LOW);// On reconnecte le moteur
  beep(2);
}


// Fonction Shoot simple
void shoot() {

  lcd.setCursor (0, 1);
  lcd.print("AUTO SHOOT");//On écrit le txt sur la 2nd ligne du LCD
  digitalWrite(shutter, HIGH); // On presse le bouton déclencheur photo
  delay (300);
  digitalWrite(shutter, LOW); // On le relache
  beep(2);
}

// Fonction Reset Compteur de pose en simulant l'ouverture du capot. (le multiexpo déduit des photos pour rien)
void resetcpt()
{
  beep(1);
  lcd.setCursor (0, 1);
  lcd.print ("Reinit counter");
  digitalWrite(capot, HIGH);
  delay (2000);
  digitalWrite(capot, LOW);
  mexposhoot(); // On lance une multiexpo pour faire croire à l'appareil que le DarkSlide est sorti, le compteur passe à 10.


}

// Fonction Sand/Timer
void fncsand(int timer)
{
  for (int i = timer; i > 0; i--)
  {
    lcd.setCursor (0, 1);
    lcd.print ("Timer:   ");
    lcd.setCursor (6, 1);
    lcd.print(i);
    beep(1);
    delay(900);
  }

}


// Initialisation Compteur


// BEEP standards
void beep (int nbrebeep) {
  for (int i = 1 ; i <= nbrebeep ; i++) {
    digitalWrite(beeper, HIGH);
    delay (50);
    digitalWrite(beeper, LOW);
    delay (50);
  }
}

// LANCEMENT SHOOT
void GO() {
  fncsand(optsand);
 // optsand = 0;
  if (optmexpo)
  {
    mexposhoot();
    optmexpo = !optmexpo;
  }
  else if (!optauto)
  {
    shootmanuel(vms[pvitesse]);
  }
  else if (optauto)
  {
    shoot();
  }
  lcdupdate();
}



void shootmanuel(int vvms) {

  lcd.setCursor (0, 1);
  lcd.print("MANUAL SHOOT");
  digitalWrite(shutalim, HIGH);
    delay(300);
  digitalWrite(shutin1, HIGH);
  delay(vvms);
  digitalWrite(shutin1, LOW);
  digitalWrite(shutin2, HIGH);
  delay(10);
  digitalWrite(shutin2, LOW);
  
  digitalWrite(shutalim, LOW);
  delay (300);
  beep(2);


}




/*------------------------------------------------------------------------*/
/*--------------------------------- LOOP ---------------------------------*/
/*------------------------------------------------------------------------*/
void loop() {

  ShutterState = digitalRead(btnshutter);
  if (ShutterState == LOW) GO();

  if (irrecv.decode(&decode_ir)) //Gestion des code Infrarouge reçus.
  {
    if (decode_ir.value != 4294967295) {

      switch (decode_ir.value) {

        case 16753245: //TOUCHE CH-
          optmexpo = !optmexpo;
          break;
        case 16736925://TOUCHE CH
          if (optsand == 0) optsand = 2;
          else if (optsand == 2)optsand = 10;
          else optsand = 0;
          break;
        case 16769565://TOUCHE CH+
          if (optflash == 0) optflash = 1;
          else if (optflash == 1)optflash = 2;
          else optflash = 0;
          break;
        case 16720605://TOUCHE <<
          resetcpt();
          break;
        case 16712445://TOUCHE >>
          GO();

          break;

        case 16761405://TOUCHE Play
          optauto = !optauto;

          break;

        case 16769055: // touche -
          optauto = false;
          if (pvitesse != 0)pvitesse = pvitesse - 1;
          break;

        case 16754775: // touche +
          optauto = false;
          if (pvitesse != 19)pvitesse = pvitesse + 1;
          break;

        case 16748655://TOUCHE EQ
          mexposhoot();
          break;
        case 16732845://TOUCHE 9
          shootmanuel(vms[pvitesse]);
          break;
        case 16730805://TOUCHE 8
          break;
        default :
          lcd.setBacklight(LOW);      // Backlight off
          delay(50);
          lcd.setBacklight(HIGH);     // Backlight on

      }
      Serial.println(decode_ir.value, DEC);
    }
    irrecv.resume(); // reçoit le prochain code
    lcdupdate();
  }

}

