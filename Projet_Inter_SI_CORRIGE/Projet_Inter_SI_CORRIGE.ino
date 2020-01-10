// Includes.
#include <HTS221Sensor.h>
#include <Wire.h>

int impulsions = 0;
float debit1 = 0;
float debit2 = 0;
unsigned long lastmillis = 0;

int afficheUneFois = 0;

HTS221Sensor  *HumTemp;
int Consigne_Temperature = 19;
int pinChaudPlus = 4;
int pinChaudMoins = 5;
int pinFroidPlus = 6;
int pinFroidMoins = 7;

int etape1 = 0;
int etape2 = 0;
int etape3 = 0;

int dureeetape3 = 60; //Pour compter 20 executions de l'etape 3 : 20 x 250ms = 5 secondes de regulation en temperature
int compteuretape3;

void setup()
{
 Serial.begin(9600); 

 // Configuration pins en sortie pour le controle des vannes
 pinMode(pinChaudPlus, OUTPUT);
 pinMode(pinChaudMoins, OUTPUT);
 pinMode(pinFroidPlus, OUTPUT);
 pinMode(pinFroidMoins, OUTPUT);

 // Aucune vanne n'est commandée pour le moment
 digitalWrite(pinChaudPlus, LOW);
 digitalWrite(pinChaudMoins, LOW);
 digitalWrite(pinFroidPlus, LOW);
 digitalWrite(pinFroidMoins, LOW);

 // Initialize I2C bus.
 Wire.begin();

// Initlialize components.
HumTemp  = new HTS221Sensor (&Wire);
HumTemp->Enable();

 attachInterrupt(0, debimetre, FALLING);
 etape1=1;
 etape2=0;
 etape3=0;

 afficheUneFois = 1;

 }


void loop()
{

  if(etape1==1)
  {
    //Etape 1 : ouverture des vannes a 50%
    if(afficheUneFois == 1)
    {
      Serial.println("  ");
      Serial.println("-------- Etape 1 : ouverture vannes à 50% (pendant 2 secondes) puis attente 1 sec -----------"); //print Etape 1.
      Serial.println("  ");      
      afficheUneFois = 0;
    }
    
    digitalWrite(pinChaudPlus, HIGH);
    digitalWrite(pinFroidPlus, HIGH);
    delay(2000);
    digitalWrite(pinChaudPlus, LOW);
    digitalWrite(pinFroidPlus, LOW);
    
    // Les 2 vannes sont ouvertes a 50% (= apres 2 secondes). On passe à l'etape 2 : controle du débit
    etape1=0;
    etape2=1;
    etape3=0;
    afficheUneFois = 1;

    delay(1000);                            //Attendre 1 sec que le debit s'établisse
  }
 
  if(etape2==1)
  {
    if(afficheUneFois == 1)
    {      
      Serial.println("  ");
      Serial.println("-------- Etape 2 : Mesure et Controle du debit jusqu'a atteindre 8 L/mn = 133 mL/s ----------"); //print Etape 2.
      Serial.println("  ");
      afficheUneFois = 0;
      impulsions = 0;                         // Remise a zero du compteur pour une nouvelle mesure
      lastmillis = millis();                  // Mise a jour du compteur de temps
      attachInterrupt(0, debimetre, FALLING); //Activation de l'interruption INT0 (sur la pin 2)          
    }
    
    // Etape 2 : mesure et controle du debit
    if (millis() - lastmillis >= 1000)
    { //Uptade every one second, this will be equal to reading frecuency (Hz).
      
      detachInterrupt(0);//Desactiver l'interruption pendant le calcul du debit
      debit1 = (impulsions * 2.2);
      //debit2 = ((debit1 * 60)/1000);
            
      Serial.print("Hz=\t"); //print the word "Hz".
      Serial.print(impulsions); //print revolutions per second or Hz. And print new line or enter.
      Serial.print("\t\t"); //Tabulation.
      Serial.print("Debit (mL/s)=\t"); //print the word "Debit".
      Serial.println(debit1); //print debit in ml per second. And print new line or enter.
      //Serial.print("\t\t"); //Tabulation.
      //Serial.print("Debit (L/mn)=\t"); //print the word "Debit".
      //Serial.println(debit2); //print debit in ml per second. And print new line or enter.

      if(debit1<133) //si le debit<133ml/s c'est à dire debit<8L/mn
      {
        // Si le debit est insuffisant (inferieur a 8 L/mn = 133 mL/s), on ouvre encore les vannes pendant 250 ms
        digitalWrite(pinChaudPlus, HIGH);
        digitalWrite(pinFroidPlus, HIGH);        
      }
      
      impulsions = 0; // Remise a zero du compteur pour une nouvelle mesure

      lastmillis = millis(); // Mise a jour du compteur de temps
      attachInterrupt(0, debimetre, FALLING); //enable interrupt
      // this code will be executed every time the interrupt 0 (pin2) gets low.
    }
    //else
    //{
    //  Serial.println(millis()); //Affiche valeur timer.      
    //}

    if( (debit1<133) && (millis() - lastmillis == 250) )   // Si debit<133mL/s et 250ms écoulées : on arrete d'ouvrir les vannes
    {
      digitalWrite(pinChaudPlus, LOW);
      digitalWrite(pinFroidPlus, LOW);  
    }
      
    if(debit1>=133) //si le debit > 133ml/s c'est à dire 8L/mn : on passe a l'etape 3
    {
        // Debit de caracterisation atteint. On passe à l'etape 3
          etape1=0;
          etape2=0;
          etape3=1;
          debit1=0;
          afficheUneFois = 1;
          detachInterrupt(0);//Arret de la detection de l'interruption INT0 (pin 2)
          impulsions = 0; // Remise a zero du compteur pour une nouvelle mesure   

          // Comme le debit est atteint, on ne commande plus les vannes pour les maintenir en position
          digitalWrite(pinChaudPlus, LOW);
          digitalWrite(pinChaudMoins, LOW);
          digitalWrite(pinFroidPlus, LOW);
          digitalWrite(pinFroidMoins, LOW);      
    }  
  }
  
  if(etape3==1)
  {
    if(afficheUneFois == 1)
    {      
      Serial.println("  ");
      Serial.println("-------- Etape 3 : 5 secondes de Regulation en T° et commande des vannes toutes les 250ms ---"); //print Etape 3.
      Serial.println("  ");
      afficheUneFois = 0;
    }
    
    //etape3 : régulation en temperature
        //if (millis() - lastmillis == 1000)
        { //Uptade every one second, this will be equal to reading frecuency (Hz).
          detachInterrupt(0);//Disable interrupt when calculating

          // Read humidity and temperature.
          float temperature;
          //HumTemp->GetHumidity(&humidity);
          HumTemp->GetTemperature(&temperature);
          Serial.print("Temp (°C)=\t"); //print the word "Debit".
          Serial.println(temperature); //print temperaure.

          if(temperature>Consigne_Temperature)
          {
            // Si T° > Consigne : Ouvrir le Froid et fermer le Chaud pendant 250ms
            digitalWrite(pinFroidPlus, HIGH);
            digitalWrite(pinChaudMoins, HIGH);
            delay(250);
            digitalWrite(pinFroidPlus, LOW);
            digitalWrite(pinChaudMoins, LOW);
          }

          if(temperature<Consigne_Temperature)
          {
            // Si T° < Consigne : Ouvrir le Chaud et fermer le Froid pendant 250ms
            digitalWrite(pinChaudPlus, HIGH);
            digitalWrite(pinFroidMoins, HIGH);
            delay(250);
            digitalWrite(pinChaudPlus, LOW);
            digitalWrite(pinFroidMoins, LOW);
          }

          if(temperature == Consigne_Temperature)
          {
            // Si T° = Consigne : ne pas commander les vannes pour les conserver à la meme position
            digitalWrite(pinChaudPlus, LOW);
            digitalWrite(pinChaudMoins, LOW);
            digitalWrite(pinFroidPlus, LOW);
            digitalWrite(pinFroidMoins, LOW);
          }

          compteuretape3++;                   // compter 20 executions de l'etape 3 : 20 x 250ms = 5 secondes de regulation en temperature
          if(compteuretape3 >= dureeetape3)   // Si 5 sec. de regulation, retour a l'etape 2
          {
            etape1=0;
            etape2=1;
            etape3=0;
            compteuretape3=0;
            afficheUneFois = 1; 
            attachInterrupt(0, debimetre, FALLING); //Activer l'interruption INT0 (pin 2)            
          }
        }
  }
  
}


 void debimetre(){
  impulsions++;
 }
