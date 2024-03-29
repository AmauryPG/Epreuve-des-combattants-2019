#include "functionsMove.h"

int zones = 0;

//               pour le robot A
double kp = 0.001;
double ki = 0.000002;
double kd = 0.018;  

//               pour le robot B
/************************************************
double kp = 0.010;
double ki = 0.0000001;
double kd = 0.018; 
************************************************/

float vitesse = 0.5;

///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////fonction action////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

//tourne sur place avec un angle et une vitesse
void TournerSurPlace(float angleEnDegreCercle, float vitesse)
{
    int moteur1 = moteurDroit;
    int moteur2 = moteurGauche;
    int direction = 1;

    if (angleEnDegreCercle < 0)
    {
        moteur1 = moteurGauche;
        moteur2 = moteurDroit;
        direction = -1;
    }

    //set distance en encodeur
    int angleEncodeur = getAngleEncodeur(angleEnDegreCercle);

    //avance les deux moteurs
    while (direction * angleEncodeur >= ENCODER_Read(moteur1) && direction * -angleEncodeur <= ENCODER_Read(moteur2))
    {
        MOTOR_SetSpeed(moteur1, vitesse);
        MOTOR_SetSpeed(moteur2, -vitesse);
    }
    //reset et arrete les moteurs avec un delay
    MOTOR_SetSpeed(moteur1, 0);
    MOTOR_SetSpeed(moteur2, 0);
}

//*********************************************************************************************************
//                                             PID general
//                                            *Pas toucher*
//*********************************************************************************************************

unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double TotalError, rateError;

void PID(double vitesse, double setPoint, double variable)
{
    currentTime = millis();                             //get current time
    elapsedTime = (double)(currentTime - previousTime); //compute time elapsed from previous computation

    error = setPoint - variable; // determine error
    TotalError += error * elapsedTime;                              // compute integral
    rateError = (error - lastError) / elapsedTime;                  // compute derivative

    double out = kp * error + ki * TotalError + kd * rateError; //PID output

    lastError = error;          //remember current error
    previousTime = currentTime; //remember current time

    MOTOR_SetSpeed(moteurDroit, out);
    MOTOR_SetSpeed(moteurGauche, vitesse);
    delay(5); //delay for the motors
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////ERREUR DE DIMENSION
void PIDAcceleration(float vitesseInitial, float vitesseFinale, float distanceCM)
{
    /****
     * x   | y
     * 100 | 109
     * 95  | 53
     * 90  | 52
     * 70  | 50
     * 50  | 50
     * 25  | 48
    */
    float constante = ((-24.5407)/(distanceCM - 100.408)) + 48.8973;
    float acceleration = constante *((pow(vitesseFinale, 2) - pow(vitesseInitial, 2)) / (2 * getDistanceEncodeur(distanceCM)));
    int temps = 1;
    float vitesseModifier = vitesseInitial + acceleration * temps;

    while (vitesseModifier <= vitesseFinale)
    {
        //moteur droit est slave et gauche est master
        PID(vitesseModifier, ENCODER_Read(moteurGauche), ENCODER_Read(moteurDroit));

        //Serial.println(vitesseModifier,7);

        temps++;
        vitesseModifier = vitesseInitial + acceleration * temps;
    } 
}
 

void PIDAvancer(float vitesseInitial, float vitesseFinale, float distanceCM, float distanceAcceleration)
{ 
    PIDAcceleration(vitesseInitial,vitesseFinale,distanceAcceleration);
    while(ENCODER_Read(moteurGauche) < getDistanceEncodeur(distanceCM))
    {
        PID(vitesseFinale,ENCODER_Read(moteurGauche),ENCODER_Read(moteurDroit));
    }
}

void PinceOpen()
{ 
    SERVO_Enable(1);
    delay(10);
    SERVO_SetAngle(1, 180);
    delay(10); 
}

void PinceClose()
{
  SERVO_Enable(1);
  delay(10);
  SERVO_SetAngle(1, 145);
  delay(10);
}

/****************************************************************************************
 *                              fonctions principaux
****************************************************************************************/
void ChercherBalle()
{
    //distribution des zones
    //0         1
    //   robot
    //2         3

    switch (zones)
    {
    case 0:

        break;
    case 1:
        //tourner 90 degre
        TournerSurPlace(90,vitesse);

        //avancer jusqu'a la ligne
        PIDAcceleration(0,vitesse,5);
        while(!digitalRead(pinCapteurGauche) && !digitalRead(pinCapteurMilieu) && !digitalRead(pinCapteurDroit))
        {
            PID(vitesse,ENCODER_Read(moteurGauche),ENCODER_Read(moteurDroit));
        }

        //tourner 90 degre
        TournerSurPlace(-90,vitesse);

        //avancer jusqu'a la ligne
        PIDAcceleration(0,vitesse,5);
        while(!digitalRead(pinCapteurGauche) && !digitalRead(pinCapteurMilieu) && !digitalRead(pinCapteurDroit))
        {
            PID(vitesse,ENCODER_Read(moteurGauche),ENCODER_Read(moteurDroit));
        }
        
        //suiveur de ligne
        while(!digitalRead(pinCapteurGauche) && !digitalRead(pinCapteurMilieu) && !digitalRead(pinCapteurDroit))
        {
            PID(vitesse,digitalRead(pinCapteurMilieu),(digitalRead(pinCapteurGauche)+digitalRead(pinCapteurDroit)));
        }
 
        break;
    case 2:
        break;
    case 3:
        break;
    }
}

// Fonction permettant de suivre une ligne
//Besoin d'être plus délicate
void SuiveurLigne() 
{

    switch (Conv_DigitalAnalog())
    {
    case 0:
        /*if(first=0){*/
        MOTOR_SetSpeed(moteurDroit, vitesseNormale);
        MOTOR_SetSpeed(moteurGauche, vitesseNormale);
        break;
    case 2:
        MOTOR_SetSpeed(moteurDroit, vitesseNormale);
        MOTOR_SetSpeed(moteurGauche, vitesseLente);
        Serial.println("case 2");
        break;
    case 4:
        MOTOR_SetSpeed(moteurDroit, vitesseNormale);
        MOTOR_SetSpeed(moteurGauche, vitesseLente);
        Serial.println("case 4");
        break;
    case 6:
        MOTOR_SetSpeed(moteurDroit, vitesseNormale);
        MOTOR_SetSpeed(moteurGauche, vitesseLente);
        Serial.println("case 6");
        break;
    case 8:
        MOTOR_SetSpeed(moteurGauche, vitesseNormale);
        MOTOR_SetSpeed(moteurDroit, vitesseLente);
        Serial.println("case 8");
        break;
    case 12:
        MOTOR_SetSpeed(moteurGauche, vitesseNormale);
        MOTOR_SetSpeed(moteurDroit, vitesseLente);
        Serial.println("case 12");
        break;
    case 14:
        MOTOR_SetSpeed(moteurDroit, 0);
        MOTOR_SetSpeed(moteurGauche, 0);
        Serial.println("case 14");
        break;

    default:
            MOTOR_SetSpeed(moteurDroit, vitesseNormale);
            MOTOR_SetSpeed(moteurGauche, vitesseNormale);
            //Serial.println("Default Value_SumCapteur");
            break;
    
    }
    
}