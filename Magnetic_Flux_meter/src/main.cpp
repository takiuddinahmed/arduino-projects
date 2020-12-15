#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Arduino_Helpers.h>
#include <AH/Hardware/FilteredAnalog.hpp>
#include <AH/Timing/MillisMicrosTimer.hpp>

// CONST
#define MAX_CAL 100
#define MT_PER_STEP 0.257 * 0.083 // devide by 12


// pin definations
#define LCD_RS_PIN 13
#define LCD_EN_PIN 12
#define LCD_D4_PIN 11
#define LCD_D5_PIN 10
#define LCD_D6_PIN 9
#define LCD_D7_PIN 8
#define SENSOR_PIN A0

// classes
LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);
// FilteredAnalog <10, 6, uint32_t> filteredSensor = SENSOR_PIN;
// variables
float sensorValue = 0.0;
float zeroPoint = 618.0;

// functions
float avgValue(int);
void calibrate();


void setup()
{
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("  Flux Density ");
  // FilteredAnalog<>::setupADC();
  // filteredSensor.invert();
}

void loop()
{
  static Timer<millis> timer = 800;

  // if(timer && filteredSensor.update()){
  if(timer){
    sensorValue = avgValue(MAX_CAL);
    sensorValue -= zeroPoint;



    // sensorValue = (sensorValue * 5.0 * 1000) / (1023.0);
    float magneticFlux = sensorValue * MT_PER_STEP;
    if (magneticFlux > -1.5 && magneticFlux < 1.5){
      magneticFlux = 0.0;
    }

    // lcd.setCursor(0, 0);
    // lcd.print("A=");
    // lcd.print(sensorValue);
    // lcd.print(" mV  ");
    lcd.setCursor(4, 1);
    lcd.print("B=");
    lcd.print(magneticFlux);
    lcd.print(" mT  ");
  }

  // float analogValue = avgValue(MAX_CAL);
  // sensorValue = (analogValue * 5.0 * 1000) / (1023.0);
  
  // lcd.setCursor(4, 1);
  // lcd.print("B=");
  // lcd.print(sensorValue);
  // lcd.print(" mT   ");
  // delay(500);
}


float avgValue(int cal = MAX_CAL){
  float avg = 0;
  for(int i=0;i<cal;i++){
    avg += analogRead(SENSOR_PIN);
    delay(10);
  }
  avg /= MAX_CAL;
  return avg;
}