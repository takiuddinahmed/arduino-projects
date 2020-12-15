#include <Arduino.h>

// define pin 
#define X_AXIS_SENSOR A0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int X_AXIS_VALUE = analogRead(X_AXIS_SENSOR);
  Serial.println(X_AXIS_VALUE);

  delay(200);
}