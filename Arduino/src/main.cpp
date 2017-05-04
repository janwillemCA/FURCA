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

char speed = 100;
unsigned int timeSetDirection;
unsigned int timeSetDrive;
unsigned int currentSpeed;

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
  timeSetDirection = millis();
  timeSetDrive = millis();
  currentSpeed = 0;
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
            case 's':
              if (BTserial.available()) {
                  speed = BTserial.read();
                  Serial.println(speed);
                }
              break;
            }
        }

      if (timeSetDirection + 250 <= millis()) {
          center();
        }
      if (timeSetDrive + 250 <= millis()) {
          hold();
        }

    }
}
void forwards(){
  timeSetDrive = millis();
  digitalWrite(drive, LOW);   // LOW forwards, HIGH backwards
  // start engine at low speed increase to higer speed
  for (char i = currentSpeed; i < speed; i++) {
      analogWrite(pwmDrive, i);
      delay(10/i);
    }
  analogWrite(pwmDrive, speed);   // PWM Speed Control
  currentSpeed = speed;
}

void backwards(){
  timeSetDrive = millis();
  digitalWrite(drive, HIGH);   // LOW forwards, HIGH backwards
  // start engine at low speed increase to higer speed
  for (char i = currentSpeed; i < speed; i++) {
      analogWrite(pwmDrive, i);
      delay(10/i);
    }
  analogWrite(pwmDrive, speed);   // PWM Speed Control
  currentSpeed = speed;
}

void left(){
  timeSetDirection = millis();
  digitalWrite(direction, HIGH); // LOW right, HIGH left
  analogWrite(pwmDirection, 255); // PWM Speed Control
}

void center(){
  analogWrite(pwmDirection, 0);        // PWM Speed Control
}

void right(){
  timeSetDirection = millis();
  digitalWrite(direction, LOW);   // LOW right, HIGH left
  analogWrite(pwmDirection, 255);       // PWM Speed Control
}

void hold(){
  analogWrite(pwmDrive, 0);   // PWM Speed Control
  currentSpeed = 0;
}
