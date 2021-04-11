#include <Arduino.h>
#include <SoftwareSerial.h>

#define RX_PIN 10
#define TX_PIN 9
// #define RX_PIN 11
// #define TX_PIN 12
#define BAUD_RATE 115200

SoftwareSerial s(RX_PIN, TX_PIN);
String data = "";

void setup()
{
  Serial.begin(9600);
  s.begin(115200);
}

void loop()
{
  if (s.available())
  {
    data = "";
    while (s.available())
    {
      data += (char) s.read();
    }
    Serial.print(data);
    data = "";
  }
  if (Serial.available())
  {
    data = "";
    while (Serial.available()){
      data += (char) Serial.read();
    }
    s.print(data);
    // Serial.print(data);
    data = "";
  }
}