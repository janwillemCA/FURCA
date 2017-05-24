/**
 * @author Steven van der Vlist
 * @author Henri van de Munt
 * @author Jan Willem Castelein
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "TimerOne.h"
#include <stdio.h>

#define PWMDRIVE 5
#define PWMSTEER 4
#define BACKWARDS 6
#define FORWARDS 7
#define LEFT 8
#define RIGHT 9

//#define PWMDRIVEVALUE 120
#define PWMSTEERVALUE 255
#define TRIGPINR 11 // right seen from driver position
#define ECHOPINR 10
#define TRIGPINL 12 // left seen from driver position
#define ECHOPINL 13

SoftwareSerial BTserial(2, 3); // RX | TX
/* Connect the HC-05 TX to Arduino pin 2 RX. */
/* Connect the HC-05 RX to Arduino pin 3 TX through a voltage divider. */

char receivedData = ' ';
bool switchPing = true;
bool holdcar = false;
char stringL[20];
char stringR[20];
int distanceL;
int distanceR;
char stringPWM[20];
bool mode = 1; //1 = autonomous. 0 = manual.
int PWMDRIVEVALUE = 120;

/* sensor variables */
int lastMeasure[3];
int measureCnt[3];
int measureDifference[3];

/* time control for acceleration */
int forwardsCounter = 0;
bool forwardsStart = false;

/* time control for steering */
int steeringCounter = 0;
bool steeringStart = false;

/* time control for backwards */
int backwardsCounter = 0;
bool backwardsStart = false;

/* time control brakes */
int brakeCounter = 0;
int rawIntData[3];
int rawIntDataCounter = 0;
bool brakeStart = false;
bool disableSwitch = false;
bool lightsOn = true;

void forwards();
void backwards();
void left();
void center();
void right();
void hold();
void processPing();
void timerInterrupt();
void improveData(int distance, int sensor);
void timingControl(bool *timingBool, int *counter, int ticks, int motor);

void setup() {
  BTserial.begin(115200);
  Serial.begin(9600);

  /* engine control */
  analogWrite(PWMDRIVE, PWMDRIVEVALUE);   // PWM Speed Control
  analogWrite(PWMSTEER, PWMSTEERVALUE);   // PWM Steer Control

  /* sensor Control */
  pinMode(TRIGPINL, OUTPUT);
  pinMode(ECHOPINL, INPUT);
  pinMode(TRIGPINR, OUTPUT);
  pinMode(ECHOPINR, INPUT);
  pinMode(A0,OUTPUT);
  digitalWrite(A0,HIGH);


  //timer initialization
  Timer1.initialize(35000);

  Timer1.attachInterrupt(timerInterrupt);
}


int testLcounter = 0;
int testRcounter = 0;
void loop(){

  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);

  if (BTserial.available()) {
      receivedData = BTserial.read();
      Serial.write(receivedData);
      if(disableSwitch == false) {
        switch (receivedData) {
          case 'W':
            digitalWrite(2, LOW);
            digitalWrite(3, LOW);
            forwards();
            break;
          case 'A':
            digitalWrite(2, LOW);
            digitalWrite(3, LOW);
            left();
            break;
          case 'S':
            digitalWrite(2, HIGH);
            digitalWrite(3, HIGH);
            backwards();
            break;
          case 'D':
            digitalWrite(2, LOW);
            digitalWrite(3, LOW);
            right();
            break;
          case 'H':         // hold
            digitalWrite(2, HIGH);
            digitalWrite(3, HIGH);
            hold();
            break;
          case 'C':           // hold
            digitalWrite(2, LOW);
            digitalWrite(3, LOW);
            center();
            break;
          case 'Z':
            mode =1;
            break;
          case 'X':
            mode =0;
            break;
          case 'L':
            digitalWrite(A0,HIGH);
            break;
          case 'O':
            digitalWrite(A0,LOW);
            break;
          case 'P':
            disableSwitch = true;
            break;
          default:
            // hold();
            // if nothing else match, do the default
            // default is optional
            break;
          }
      } else {
        if(rawIntDataCounter < 3) {
          if(receivedData >= 48 && receivedData <= 57) {
            rawIntData[rawIntDataCounter] = receivedData - '0';
            rawIntDataCounter++;
          }
        } else {
          rawIntDataCounter = 0;
          disableSwitch = false;
          int finalData = rawIntData[2] + rawIntData[1]*10 + rawIntData[0]*100;
          if(finalData <= 255) {
            PWMDRIVEVALUE = finalData;
          }
          Serial.print("PWMDRIVEVALUE:");
          Serial.println(PWMDRIVEVALUE);
        }
      }
    }
}

void forwards(){
  BTserial.write("V200");
  holdcar = false;
  analogWrite(PWMDRIVE, 200);
  forwardsStart = true;
  digitalWrite(FORWARDS, HIGH);
  digitalWrite(BACKWARDS, LOW);
}

void backwards(){
  BTserial.write("V200");
  analogWrite(PWMDRIVE, 200);
  backwardsStart = true;
  digitalWrite(FORWARDS, LOW);
  digitalWrite(BACKWARDS, HIGH);
}

void left(){
  if(mode == 0) {
    steeringStart = true;
  }
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, LOW);
}

void center(){
  digitalWrite(LEFT, LOW);
  digitalWrite(RIGHT, LOW);
}

void right(){
  if(mode == 0) {
    steeringStart = true;
  }
  digitalWrite(RIGHT, HIGH);
  digitalWrite(LEFT, LOW);
}

void hold(){
  if(holdcar == false) {
    analogWrite(PWMDRIVE, 255);   // PWM Speed Control
    BTserial.write("V255");
    brakeStart = true;
    digitalWrite(FORWARDS, LOW);
    digitalWrite(BACKWARDS, HIGH);
    holdcar = true;
  }
}

void timerInterrupt() {
  if(mode == 1) {
    processPing();
  }

//Serial.println(steeringCounter);
//Serial.println(steeringStart);
timingControl(&forwardsStart,&forwardsCounter, 30, 1);
timingControl(&backwardsStart,&backwardsCounter, 30, 1);
timingControl(&steeringStart,&steeringCounter, 30, 2);
timingControl(&brakeStart,&brakeCounter, 8  ,3);

  // if(forwardsStart == true){
  //   forwardsCounter++;
  // }
  //
  // if(forwardsCounter == 10){
  //   analogWrite(PWMDRIVE, PWMDRIVEVALUE);
  //   forwardsCounter = 0;
  //   forwardsStart = false;
  // }
}

void improveData(int distance, int sensor) {
  int maximumDifference = 0;
  if(PWMDRIVEVALUE <= 160) {
    maximumDifference = PWMDRIVEVALUE / 10;
  } else {
    maximumDifference = PWMDRIVEVALUE / 7;
  }

  //sensor 0 = left, sensor 1 = right, sensor 2 = empty
  if (distance <= 120) {
      if(measureCnt[sensor] == 0) {
        lastMeasure[sensor] = distance;
        measureCnt[sensor]++;
      }
      if(measureCnt[sensor] >= 1 && measureCnt[sensor] <8) {
        if(distance > lastMeasure[sensor]) {
          measureDifference[sensor] = distance - lastMeasure[sensor];
        } else {
          measureDifference[sensor] = lastMeasure[sensor] - distance;
        }

        //Serial.print("lastMeasurediff sensor ");Serial.print(sensor);Serial.print(":");
        //Serial.println(measureDifference[sensor]);

        if(measureDifference[sensor] <= maximumDifference) {
          lastMeasure[sensor] = distance;
          measureCnt[sensor]++;
        } else if (measureDifference[sensor] > 10) {
          measureCnt[sensor] = 0;
          //Serial.println("set to 0");
        }

      }

      if(measureCnt[sensor] == 4) {
        if(distance != 0) {
          if(sensor == 0) {
            //Serial.print("left: ");
            sprintf(stringL,"L%d",distance);
            distanceL = distance;
            //Serial.println(stringL);
            BTserial.print(stringL);
          }else if(sensor == 1){
            //Serial.print("right: ");
            sprintf(stringR,"R%d",distance);
            distanceR = distance;
            //Serial.println(stringR);
            BTserial.print(stringR);
          }else {
          }
        }
        measureCnt[sensor] = 0;
      }

    }
}

void timingControl(bool *timingBool, int *counter, int ticks, int motor) {
  if(*timingBool == 1){
    *counter = *counter +1;
    if(*counter == ticks){
      //what to do when timer runs out for different motors
      if(motor == 1){
        //forwards/backwards motor
        analogWrite(PWMDRIVE, PWMDRIVEVALUE);
        sprintf(stringPWM,"V%d",PWMDRIVEVALUE);
        BTserial.write(stringPWM);
      }else if(motor == 2){
        //steering motor
        center();
      }else if(motor == 3){
        //brake motor
        analogWrite(PWMDRIVE, PWMDRIVEVALUE);
        sprintf(stringPWM,"V%d",0);
        BTserial.write(stringPWM);
        digitalWrite(BACKWARDS, LOW);
      }
      *counter = 0;
      *timingBool = false;
    }
  }
}

void processPing() {
  if(switchPing == true) {
    //rightsensor
    long durationR, distanceR;
    digitalWrite(TRIGPINR, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGPINR, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPINR, LOW);
    durationR = pulseIn(ECHOPINR, HIGH);
    distanceR = (durationR/2) / 29.1;
    switchPing =  false;

    improveData(distanceR, 1);

  } else if(switchPing == false) {
    //leftsensor
    long durationL, distanceL;
    digitalWrite(TRIGPINL, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGPINL, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPINL, LOW);
    durationL = pulseIn(ECHOPINL, HIGH);
    distanceL = (durationL/2) / 29.1;
    switchPing = true;
    //Serial.println(distanceL);
    improveData(distanceL, 0);
  }
}
