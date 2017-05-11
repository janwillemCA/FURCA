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

#define PWMDRIVEVALUE 100
#define PWMSTEERVALUE 255

#define TRIGPINR 11 // right seen from driver position
#define ECHOPINR 10
#define TRIGPINL 13 // left seen from driver position
#define ECHOPINL 12


SoftwareSerial BTserial(2, 3); // RX | TX
// Connect the HC-05 TX to Arduino pin 2 RX.
// Connect the HC-05 RX to Arduino pin 3 TX through a voltage divider.

char receivedData = ' ';
bool switchPing = true;
bool holdcar = false;
char stringL[20];
char stringR[20];
int lastMeasure;
int measureCnt = 0;
int measureDifference = 0;
int forwardsCounter = 0;
bool forwardsStart = false;

void forwards();
void backwards();
void left();
void center();
void right();
void hold();
void processPing();
void timerInterrupt();

void setup() {
  BTserial.begin(115200);
  Serial.begin(9600);

  // engine control
  analogWrite(PWMDRIVE, PWMDRIVEVALUE);   // PWM Speed Control
  analogWrite(PWMSTEER, PWMSTEERVALUE);   // PWM Steer Control

  // sensor Control
  //pinMode(TRIGPINL, OUTPUT);
  //pinMode(ECHOPINL, INPUT);
  pinMode(TRIGPINR, OUTPUT);
  pinMode(ECHOPINR, INPUT);

  //timer initialization
  Timer1.initialize(50000);
  Timer1.attachInterrupt(timerInterrupt);
}

void loop(){



  if (BTserial.available()) {
      receivedData = BTserial.read();
      Serial.write(receivedData);
      switch (receivedData) {
        case 'W':
          forwards();
          break;
        case 'A':
          left();
          break;
        case 'S':
          backwards();
          break;
        case 'D':
          right();
          break;
        case 'H':         // hold
          hold();
          break;
        case 'C':           // hold
          center();
          break;
        default:
          // hold();
          // if nothing else match, do the default
          // default is optional
          break;
        }
    }
}

void forwards(){
  holdcar = false;
  analogWrite(PWMDRIVE, 200);
  forwardsStart = true;
  digitalWrite(FORWARDS, HIGH);
  digitalWrite(BACKWARDS, LOW);
}

void backwards(){
  digitalWrite(FORWARDS, LOW);
  digitalWrite(BACKWARDS, HIGH);
}

void left(){
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, LOW);
}

void center(){
  digitalWrite(LEFT, LOW);
  digitalWrite(RIGHT, LOW);
}

void right(){
  digitalWrite(RIGHT, HIGH);
  digitalWrite(LEFT, LOW);
}

void hold(){
  if(holdcar == false) {
    analogWrite(PWMDRIVE, 255);   // PWM Speed Control
    digitalWrite(FORWARDS, LOW);
    digitalWrite(BACKWARDS, HIGH);
    delay(500);
    digitalWrite(BACKWARDS, LOW);
    analogWrite(PWMDRIVE, PWMDRIVEVALUE);   // PWM Speed Control
    holdcar = true;
  }


}

void timerInterrupt() {
  processPing();

  if(forwardsStart == true){
    forwardsCounter++;
  }

  if(forwardsCounter == 15){
    analogWrite(PWMDRIVE, PWMDRIVEVALUE);
    forwardsCounter = 0;
    forwardsStart = false;
  }
}

void processPing() {
  //if(switchPing == true) {
    //rightsensor
    long durationR, distanceR;
    digitalWrite(TRIGPINR, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGPINR, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPINR, LOW);
    durationR = pulseIn(ECHOPINR, HIGH);
    distanceR = (durationR/2) / 29.1;
    //Serial.println(distanceR);
    if (distanceR <= 120) {
        if(measureCnt == 0) {
          lastMeasure = distanceR;
          measureCnt++;
        }
        if(measureCnt >= 1 && measureCnt <4) {
          if(distanceR > lastMeasure) {
            measureDifference = distanceR - lastMeasure;
          } else {
            measureDifference = lastMeasure - distanceR;
          }

          Serial.print("lastMeasurediff: ");
          Serial.println(measureDifference);
          //Serial.println(measureCnt);

          if(measureDifference <= 20) {
            lastMeasure = distanceR;
            measureCnt++;
          } else if (measureDifference > 20) {
            measureCnt = 0;
            //Serial.println("set to 0");
          }

        }

        if(measureCnt == 4) {
          if(distanceR != 0) {
            sprintf(stringR,"M%d",distanceR);
            Serial.println(stringR);
            BTserial.print(stringR);
          }
          measureCnt = 0;
        }

      }
    //switchPing =  false;

  //} else if(switchPing == false) {
    //leftsensor
    // long durationL, distanceL;
    // digitalWrite(TRIGPINL, LOW);
    // delayMicroseconds(2);
    // digitalWrite(TRIGPINL, HIGH);
    // delayMicroseconds(10);
    // digitalWrite(TRIGPINL, LOW);
    // durationL = pulseIn(ECHOPINL, HIGH);
    // distanceL = (durationL/2) / 29.1;
    // if (distanceL <= 120) {
    //   sprintf(stringL,"L%d",distanceL);
    //   Serial.println(stringL);
    //   BTserial.print(stringL);
    //   }
    // switchPing = true;
  //}

}
