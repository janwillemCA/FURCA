/**
 * @author Steven van der Vlist
 * @author Henri van de Munt
 * @author Jan Willem Castelein
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

#define PWMDRIVE 4
#define PWMSTEER 5
#define BACKWARDS 6
#define FORWARDS 7
#define LEFT 8
#define RIGHT 9

#define TRIGPINR 13 // right seen from driver position
#define ECHOPINR 12
#define TRIGPINL 11 // left seen from driver position
#define ECHOPINL 10


SoftwareSerial BTserial(2, 3); // RX | TX
// Connect the HC-05 TX to Arduino pin 2 RX.
// Connect the HC-05 RX to Arduino pin 3 TX through a voltage divider.

char receivedData = ' ';

void forwards();
void backwards();
void left();
void center();
void right();
void hold();

void setup() {
  BTserial.begin(115200);
  Serial.begin(9600);

  // engine control
  analogWrite(PWMDRIVE, 150);   // PWM Speed Control
  analogWrite(PWMSTEER, 255);   // PWM Steer Control

  // sensor Control
  pinMode(TRIGPINL, OUTPUT);
  pinMode(ECHOPINL, INPUT);
  pinMode(TRIGPINR, OUTPUT);
  pinMode(ECHOPINR, INPUT);
}

void loop(){
  long durationR, distanceR;
  long durationL, distanceL;
  digitalWrite(TRIGPINR, LOW);
  digitalWrite(TRIGPINL, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPINR, HIGH);
  digitalWrite(TRIGPINL, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPINR, LOW);
  digitalWrite(TRIGPINL, LOW);
  durationR = pulseIn(ECHOPINR, HIGH);
  durationL = pulseIn(ECHOPINL, HIGH);
  distanceR = (durationR/2) / 29.1;
  distanceL = (durationL/2) / 29.1;
  if (!(distanceR >= 450) || !(distanceR <= 0)) {
      Serial.print(distanceR);
      Serial.println("R cm");
      BTserial.println("R");
      BTserial.println(distanceR);
    }
  if (!(distanceL >= 450) || !(distanceL <= 0)) {
      Serial.print(distanceL);
      Serial.println("L cm");
      BTserial.println("L");
      BTserial.println(distanceL);
    }


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
  digitalWrite(FORWARDS, LOW);
  digitalWrite(BACKWARDS, LOW);
}
