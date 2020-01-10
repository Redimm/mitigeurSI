void setup() {
                                     // à rajouter au code principal
  public int pinDebit = 10;
  pinMode(pinDebit, INPUT)
 
}


float lireDebit (int tmpLecture)         //temps de lecture en ms
{
  float tmp = tmpLecture;
  float debit = 0;
  int impParLitre = 450;               // 2.22 mL par impultion
  bool etatPin;                       //etat actuel du pin débit
  bool etatPinPrev = false;           //état d'avant du pin débit
  int compteurImp = 0;

  do{
     etatPin = digitalRead(pinDebit);
         if (etatPin){
          if (!etatPinPrev){
            compteurImp++;
            etatPinPrev = true;
            }     
      } else {etatPinPrev = false;}
     
    delay(1);
    tmp = tmp - 1;
    }
  while (tmp >= 0);

  
  debit = compteurImp * (1/impParLitre)*(1000/tmpLecture)*60;
  return(debit);              // sortie de fonction en L/min
}









