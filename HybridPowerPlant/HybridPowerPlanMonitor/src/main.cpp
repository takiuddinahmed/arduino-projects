#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_INA219.h>

// classes
LiquidCrystal lcd (6,7,8,9,10,11);
Adafruit_INA219 solarPowerSensor;
Adafruit_INA219 windPowerSensor(0x41);

void setup()
{
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.clear();
  Wire.begin();

  if (solarPowerSensor.begin()){
    Serial.println("Failed");
  }

    // windPowerSensor.begin();

    Serial.println("start");
  lcd.print("Start");
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0,0);
  lcd.print("Solar Power");
  float solarPower = solarPowerSensor.getPower_mW();
  lcd.print(solarPower);
  lcd.print(" mW");
  Serial.println(solarPower);
  delay(500);
}