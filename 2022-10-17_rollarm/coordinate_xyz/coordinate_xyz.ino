/* 
 *  VEDI LA LEZIONE COMPLETA : https://youtu.be/7sVEfta5QTM
 */



/* -----------------------------------------------------------------------------
  Author             : Allen
  Check              : Amy
  Version            : V2.0
  Date               : 12/09/2019
  Description        : Rollarm control program
  Company website    : http://www.sunfounder.com
   ---------------------------------------------------------------------------*/

/* Include ------------------------------------------------------------------*/
// Create servo object to control a servo. 
#include <Servo.h> 

Servo Servo_0;   /* asse 1, angolo Beta */
Servo Servo_1;   /* asse 2, angolo Alfa */
Servo Servo_2;   /* asse 3, angolo Gamma */
Servo Servo_3;

#define DATA_PRINT //Uncomment only this to print the data.

//Record the data.
int SensVal[4] = {0}; 

int Joint0[50] = {0}; 
int Joint1[50] = {0};
int Joint2[50] = {0};
int Joint3[50] = {0};

int Dif0[50] = {0}; 
int Dif1[50] = {0};
int Dif2[50] = {0};
int Dif3[50] = {0};

int KeyValue = 0;
int Time = 0;
int M0 = 0, M1 = 0, M2 = 0, M3 = 0;

/*
 - setup function
 * ---------------------------------------------------------------------------*/
void setup() 
{
  //Start the serial for debug.
  Serial.begin(9600);
  
  //Attach the servos on pins to the servo object
  Servo_0.attach(4);
  Servo_1.attach(5);
  Servo_2.attach(6);
  Servo_3.attach(7);
  
  //Set the pin 3 to input
  pinMode(3, INPUT);
  
  //Read the current value of the potentiometer and map it to the angle value.
  ReadPot();
  Mapping0();
  
  //Record the current value of the potentiometer
  M0 = SensVal[0];
  M1 = SensVal[1];
  M2 = SensVal[2];
  M3 = SensVal[3];
}

/*
 - loop function
 * ---------------------------------------------------------------------------*/
void loop() 
{
 //Print the data.
#ifdef DATA_PRINT
  ReadPot();

/*  
  Serial.print(SensVal[0]);
  Serial.print(",");
  Serial.print(SensVal[1]);
  Serial.print(",");
  Serial.println(SensVal[2]);
*/

  float alfa, beta, gamma;
  alfa  = map(SensVal[1], 0, 1023, 18000,    0) / 100.0;
  beta  = map(SensVal[0], 0, 1023, -9000, 9000) / 100.0;
  gamma = map(SensVal[2], 0, 1023, 18000,    0) / 100.0;

/*
  Serial.print(alfa);
  Serial.print(",");
  Serial.print(beta);
  Serial.print(",");
  Serial.println(gamma);
*/

  /* trasformo da gradi a radianti */
  alfa  = alfa * 3.141592/180.00; 
  beta  = beta * 3.141592/180.00; 
  gamma = gamma * 3.141592/180.00; 
  
  float xO, yO, zO;
  xO = 0;
  yO = 0;
  zO = 0;

  float xO1, yO1, zO1;
  xO1 = xO + 0;
  yO1 = yO + 0;
  zO1 = zO + 64;

  float xO2, yO2, zO2;
  xO2 = xO1 + 60*cos(alfa)*cos(beta);
  yO2 = yO1 + 60*cos(alfa)*sin(beta);
  zO2 = zO1 + 60*sin(alfa);
  
  float xP,yP,zP;
  xP = xO2 + 110*cos(alfa-gamma)*cos(beta);
  yP = yO2 + 110*cos(alfa-gamma)*sin(beta);
  zP = zO2 + 110*sin(alfa-gamma);
  
  Serial.print(xP);
  Serial.print(",");
  Serial.print(yP);
  Serial.print(",");
  Serial.println(zP);

  
#endif 

  //Check the button.
  static int Flag = 1;
  Button();
  
  //The time of pressing the button is not long then record the action.
  if ((KeyValue < 10) && (KeyValue > 0))
  {
    KeyValue = 0;
    Record();
    Mapping1(); 
  }
  //Long press the button and open the auto mode ,start repeating the action.
  else if (KeyValue > 10)
  {
    if (Flag == 1)
    {
      Flag = 0;
      Calculate();
    }
    Drive_init();
    delay(3000);
    for (int i = 1; i < Time; i++)
    {
      Drive_repeat(i);
      delay(500);
    }
  }
  //Did not press the button , open the manual mode.
  else
  {
    ReadPot();
    Mapping0();
    
    //The first axis.   
    if ((SensVal[0] - M0) >= 0)
    {
      for (; M0 <= SensVal[0]; M0++)
      {
        Servo_0.write(M0); delay(2);
      }
    }
    else
    {
      for (; M0 > SensVal[0]; M0--)
      {
        Servo_0.write(M0);  delay(2);
      }
    }
    
    //The second axis.    
    if ((SensVal[1] - M1) >= 0)
    {
      for (; M1 <= SensVal[1]; M1++)
      {
        Servo_1.write(M1);  delay(2);
      }
    }
    else
    {
      for (; M1 > SensVal[1]; M1--)
      {
        Servo_1.write(M1);  delay(2);
      }
    }
    
    //The third axis. 
    if ((SensVal[2] - M2) >= 0)
    {
      for (; M2 <= SensVal[2]; M2++)
      {
        Servo_2.write(M2);  delay(2);
      }
    }
    else
    {
      for (; M2 > SensVal[2]; M2--)
      {
        Servo_2.write(M2);  delay(2);
      }
    }
    
    //The fourth axis.    
    if ((SensVal[3] - M3) >= 0)
    {
      for (; M3 <= SensVal[3]; M3++)
      {
        Servo_3.write(M3);  delay(2);
      }
    }
    else
    {
      for (; M3 > SensVal[3]; M3--)
      {
        Servo_3.write(M3);  delay(2);
      }
    }
    
    //Record the current value of the potentiometer
    M0 = SensVal[0];
    M1 = SensVal[1];
    M2 = SensVal[2];
    M3 = SensVal[3];
    delay(10);
  }
}
