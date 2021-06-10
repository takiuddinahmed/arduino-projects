#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

// Data wire is plugged into port 4 on the Arduino
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal lcd(6,7,8,9,10,11);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

String sensor_title[4] = {"tip","inlet","outlet","wall"};

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  lcd.begin(16,2);

  // Start up the library
  sensors.begin();

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();

}

void loop(void)
{
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Loop through each device, print out temperature data
  Serial.print("START;");
  for (int i = 0; i < numberOfDevices; i++)
  {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i))
    {

      // Print the data
      float tempC = sensors.getTempC(tempDeviceAddress);
      Serial.print(sensor_title[i]);
      Serial.print(":");
      Serial.print(tempC);
      Serial.print(";");
      lcd.print(sensor_title[i].substring(0,1));
      lcd.print(":");
      lcd.print(tempC);
      lcd.print(" ");
      
    }
  }
  Serial.println("END");
  delay(500);
}
