/**
 * @author Steven van der Vlist
 * @author Henri van de Munt
 * @author Jan Willem Castelein
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "TimerOne.h"
#include <stdio.h>

#define PWMDRIVE 4
#define PWMSTEER 5
#define BACKWARDS 6
#define FORWARDS 7
#define LEFT 8
#define RIGHT 9

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

void forwards();
void backwards();
void left();
void center();
void right();
void hold();
void processPing();

void setup() {
  BTserial.begin(115200);
  Serial.begin(9600);

  // engine control
  analogWrite(PWMDRIVE, 80);   // PWM Speed Control
  analogWrite(PWMSTEER, 255);   // PWM Steer Control

  // sensor Control
  pinMode(TRIGPINL, OUTPUT);
  pinMode(ECHOPINL, INPUT);
  pinMode(TRIGPINR, OUTPUT);
  pinMode(ECHOPINR, INPUT);

  //timer initialization
  Timer1.initialize(100000);
  Timer1.attachInterrupt(processPing);
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
    analogWrite(PWMDRIVE, 80);   // PWM Speed Control
    holdcar = true;
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
    if (!(distanceR >= 450) || !(distanceR <= 0)) {
        sprintf(stringR,"R%d",distanceR);
        Serial.println(stringR);
        BTserial.print(stringR);
      }
    switchPing =  false;

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
    if (!(distanceL >= 450) || !(distanceL <= 0)) {
      sprintf(stringL,"L%d",distanceL);
      Serial.println(stringL);
      BTserial.print(stringL);
      }
    switchPing = true;
  }

}
