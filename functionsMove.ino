#include "functionsMove.h"

//               pour le robot A
double kp = 0.010;
double ki = 0.0000001;
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
    float vitesseModifier = vitesseInitial + acceleration;

    while (vitesseModifier <= vitesseFinale)
    {
        //moteur droit est slave et gauche est master
        PID(vitesseModifier, ENCODER_Read(moteurGauche), ENCODER_Read(moteurDroit));

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

///////////////////////////////////////////////////////////////////////////////////reste a tester
void PIDSuiveurLigne(float vitesse)
{
    while(digitalRead(pinCapteurGauche) || digitalRead(pinCapteurMilieu) || digitalRead(pinCapteurDroit))
    {
        PID(vitesse,digitalRead(pinCapteurMilieu),(2*digitalRead(pinCapteurGauche)+4*digitalRead(pinCapteurDroit)));
    } 
}

/****************************************************************************************
 *                              fonctions principaux
****************************************************************************************/
void ChercherBalle(int zone)
{
    //distribution des zones
    //0         1
    //   robot
    //2         3

    switch (zone)
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
        PIDSuiveurLigne(vitesse);
        
        //avancer jusqu'au mur et prendre la ball en meme temps
        PIDAvancer(vitesse,vitesse,64.9,0);
 
        //prendre la balle
        PinceClose();

        /**************************************ramenez la balle au centre**************************************/
        
        //reculer assez pour faire demi-tour
        PIDAcceleration(0,-vitesse,15);

        //demi tour
        TournerSurPlace(180,vitesse);

        //avancer jusqu'a la ligne et meme la deplacer
        PIDAvancer(vitesse,vitesse,66,0);

        //suiveur de ligne
        PIDSuiveurLigne(vitesse);

        //avancer jusqu'a la ligne et meme la deplacer
        PIDAvancer(vitesse,vitesse,5,0);

        //ouvrir la pince
        PinceOpen();

        //reculer pour faire place au prochain robot
        PIDAcceleration(0,-vitesse,15);

        break;
    case 2:
        break;
    case 3:
        break;
    }
}