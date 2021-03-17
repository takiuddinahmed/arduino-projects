// include libraries
#include <Arduino.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

// const
#define EMERGENCY_STRING "EMERGENCY"

// Pin Conf
#define radioTransmitterPin 12
#define radioTransmissionSpeed 2000
#define emergencyBtnPin 4


// classes
MPU6050 accelerometer;
RH_ASK radioTransmitter(radioTransmissionSpeed, radioTransmitterPin, radioTransmitterPin, 0);

struct SendData 
{
  /* data */
  int x;
  int y;
};

SendData sendData;


// Define Values
#define MAX_AX 17000
#define MIN_AX -17000
#define MAX_AY 17000
#define MIN_AY -17000
#define DEBUG_DELAY 100

// Variables
int16_t accX, accY, accZ;
int16_t gyroX, gyroY, gyroZ;
String commandString = "";
long long debugOldTime = 0;

// functions
void displayData();
void transmitData(String);
void checkEmergencyBtn();


void setup()
{
  Serial.begin(9600);

  // make sure I2C bus run
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  // initialization
  accelerometer.initialize();
  Serial.println("Testing device connections...");
  Serial.println(accelerometer.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  if(!radioTransmitter.init()){
    Serial.println("Radio init failed");
  }

  // pinmode
  pinMode(emergencyBtnPin, INPUT_PULLUP);

}

void loop()
{
  
  if(millis()-debugOldTime >= DEBUG_DELAY){
    checkEmergencyBtn();
    // read accelerometer data
    accelerometer.getMotion6(&accX, &accY, &accZ, &gyroX, &gyroY, &gyroZ);
    sendData.x = map(accX, MIN_AX, MAX_AX, -255, 255);
    sendData.y = map(accY, MIN_AY, MAX_AY, -255, 255);

    displayData();
    Serial.print("X=");
    Serial.print(sendData.x);
    Serial.print("   Y=");
    Serial.println(sendData.y);
    debugOldTime = millis();

    checkEmergencyBtn();

    // calculate
    commandString = "x="+ String(sendData.x) + ";y="+ String(sendData.y)+ ";     ";
    Serial.println(commandString);
    transmitData(commandString);
  }
  checkEmergencyBtn();
  delay(2);
}


void transmitData (String data){
  radioTransmitter.send((uint8_t *)data.c_str(), commandString.length());
  radioTransmitter.waitPacketSent();
}

void checkEmergencyBtn(){
  bool state = digitalRead(emergencyBtnPin);
  if(state == 0){
    Serial.println(EMERGENCY_STRING);
    transmitData(EMERGENCY_STRING);
    delay(200);
  }
}


void displayData (){
  Serial.print("ax: "); Serial.print(accX);
  Serial.print(";  ay: "); Serial.print(accY);
  Serial.print(";  az: "); Serial.print(accZ);
  Serial.print(";  gx: "); Serial.print(gyroX);
  Serial.print(";  gy: "); Serial.print(gyroY);
  Serial.print(";  gz: "); Serial.print(gyroZ);
  Serial.println();
}

