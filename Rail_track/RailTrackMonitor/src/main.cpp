#include <Arduino.h>
// #include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <ThingSpeak.h>
#include <SD.h>
#include "secret.h"

// configs
char ssid[] = WIFI_SSID;
char password[] = WIFI_PASS;
int voltageField = 1;
int currentField = 2;
int powerField = 3;
int frequencyField = 4;
String path = "/data";



// classes
WiFiClient wclient;
Adafruit_INA219 ina219;

//functions
void setup_wifi();
void reconnect();
void upload_data();
void display();
void readPower();
void reset_data();

// variables
bool readDataState = false;
bool uploadDataState = false;
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;





void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);


  setup_wifi();
  ThingSpeak.begin(wclient);

  Wire.begin(D3,D2);

  while(!ina219.begin()){
    // notify sensor not working
    Serial.println("Sensor init failed");
    delay(100);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
  readPower();
  if(loadvoltage>1.2){
    display();
    upload_data();
    reset_data();
  }
  else{
    delay(20);
  }
}

void readPower(){
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  if (current_mA < 0)
    current_mA *= -1;
}

void reset_data(){
  shuntvoltage = 0;
  busvoltage = 0;
  current_mA = 0;
  loadvoltage = 0;
  power_mW = 0;
}


void display(){
  Serial.print("Load Voltage: ");
  Serial.print(loadvoltage);
  Serial.print(" V   ");
  Serial.print("Current: ");
  Serial.print(current_mA);
  Serial.print(" mA     ");
  Serial.print("Power:  ");
  Serial.print(power_mW);
  Serial.println(" mW  ");
}


void upload_data(){
  Serial.println("Upload start....");
  long long timestamp = millis();
  ThingSpeak.setField(1, loadvoltage);
  ThingSpeak.setField(2, current_mA);
  ThingSpeak.setField(3, power_mW);
  if(ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_API_KEY) == 200)
  {
    Serial.println("Uploaded");
    long time = millis() - timestamp;
    Serial.print("Upload finis. Time : ");
    Serial.println(time);
    Serial.println("--------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("Failed");
    long time = millis() - timestamp;
    Serial.print("Upload finis. Time : ");
    Serial.println(time);
    Serial.println("--------------------------");
    Serial.println();
  }
  
}

// Connect to WiFi network
void setup_wifi()
{
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  Serial.println("Try");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

