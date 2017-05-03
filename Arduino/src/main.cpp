#include <Arduino.h>
#include <SoftwareSerial.h>

int pwmDrive = 5;    // pwm drive
int drive = 4;
int pwmDirection = 6;          // pwm direction
int direction = 7;

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
  pinMode(drive, OUTPUT);
  pinMode(direction, OUTPUT);
  BTserial.begin(115200);
  Serial.begin(9600);
}

void loop(){
  while (1) {
      if (BTserial.available()) {
          receivedData = BTserial.read();
          Serial.write(receivedData);
          switch (receivedData) {
            case 'c':
              forwards();
              break;
            case 'a':
              left();
              break;
            case 'd':
              backwards();
              break;
            case 'b':
              right();
              break;
            case 'f':   // hold
              hold();
              break;
            default:
              // hold();
              // if nothing else match, do the default
              // default is optional
              break;
            }
        }
    }
}
void forwards(){
  digitalWrite(drive, LOW);   // LOW forwards, HIGH backwards
  // start engine at low speed increase to higer speed
  for (size_t i = 40; i < 120; i++) {
      analogWrite(pwmDrive, i);
      delay(10);
    }
  analogWrite(pwmDrive, 120);   // PWM Speed Control
}

void backwards(){
  digitalWrite(drive, HIGH);   // LOW forwards, HIGH backwards
  // start engine at low speed increase to higer speed
  for (size_t i = 40; i < 120; i++) {
      analogWrite(pwmDrive, i);
      delay(10);
    }
  analogWrite(pwmDrive, 120);   // PWM Speed Control
}

void left(){
  digitalWrite(direction, HIGH); // LOW right, HIGH left
  analogWrite(pwmDirection, 255); // PWM Speed Control
}

void center(){
  analogWrite(pwmDirection, 0);        // PWM Speed Control
}

void right(){
  digitalWrite(direction, LOW);   // LOW right, HIGH left
  analogWrite(pwmDirection, 255);       // PWM Speed Control
}

void hold(){
  analogWrite(pwmDrive, 0);   // PWM Speed Control
  analogWrite(pwmDirection, 0);   // PWM Speed Control
}
