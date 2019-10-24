/*#include "functions.h"

void setup()
{
  BoardInit();

  pinMode(22, INPUT);
  pinMode(24, INPUT);
  pinMode(26, INPUT); 

  pinMode(pinCapteurDroit, INPUT);
  pinMode(pinCapteurMilieu, INPUT);
  pinMode(pinCapteurGauche, INPUT);

  Serial.begin(9600);

}

void loop()
{


  PID(0.2); 
  MOTOR_SetSpeed(moteurDroit,0);
  MOTOR_SetSpeed(moteurGauche,0);
  delay(5000);
 
  Serial.println(getDistanceInfrarouge(ROBUS_ReadIR(3)));
  delay(500);
}
*/
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
#define redpin 3
#define greenpin 5
#define bluepin 6
// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground

// set to false if using a common cathode LED
#define commonAnode true

// our RGB -> eye-recognized gamma color
byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup()
{
  Serial.begin(9600);
  Serial.println("Color View Test!");

  if (tcs.begin())
  {
    Serial.println("Found sensor");
  }
  else
  {
    Serial.println("No TCS34725 found ... check your connections");
    while (1)
      ; // halt!
  }

  // use these three pins to drive an LED

  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i = 0; i < 256; i++)
  {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;

    if (commonAnode)
    {
      gammatable[i] = 255 - x;
    }
    else
    {
      gammatable[i] = x;
    }
    //Serial.println(gammatable[i]);
  }
}

void loop()
{
  uint16_t clear, red, green, blue;

  tcs.setInterrupt(false); // turn on LED

  delay(60); // takes 50ms to read

  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true); // turn off LED

  Serial.print("C:\t");
  Serial.print(clear);
  Serial.print("\tR:\t");
  Serial.print(red);
  Serial.print("\tG:\t");
  Serial.print(green);
  Serial.print("\tB:\t");
  Serial.print(blue);

  // Figure out some basic hex code for visualization
  uint32_t sum = clear;
  float r, g, b;
  r = red;
  r /= sum;
  g = green;
  g /= sum;
  b = blue;
  b /= sum;

  r *= 256;
  g *= 256;
  b *= 256;
  
  Serial.print("\t");
  Serial.print((int)r, HEX);
  Serial.print((int)g, HEX);
  Serial.print((int)b, HEX);
  Serial.println();

  delay(100);
  //Serial.print((int)r ); Serial.print(" "); Serial.print((int)g);Serial.print(" ");  Serial.println((int)b );
}
