// headers
#include <Arduino.h>
#include <SoftwareSerial.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <TinyGPS++.h>
#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
  #include <SPI.h> // Not actually used but needed to compile
#endif
#include <HCSR04.h>

// define
#define radioSpeed 2000
#define radioRcvPin 2
#define RADIO_MAX_MSG_LEN 2
#define GSM_RX_PIN 10
#define GSM_TX_PIN 9
#define GPS_RX_PIN 11
#define GPS_TX_PIN 12
#define DEBUG_SERIAL_BAUD 9600
#define GSM_SERIAL_BAUD 115200
#define GPS_SERIAL_BAUD 4800
#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 512
#endif
#define RIGHT_MOTOR_1 10
#define RIGHT_MOTOR_2 11
#define RIGHT_MOTOR_SPEED 12
#define LEFT_MOTOR_1 9
#define LEFT_MOTOR_2 8
#define LEFT_MOTOR_SPEED 7
#define FORWARD_COMMAND_SPEED_MIN 50
#define TURN_COMMAND_SPEED_MIN 50
#define MOTION_DELAY 100

#define SONAR_TRIG_PIN 1
#define SONAR_ECHO_PIN 2
#define MAX_ACCEPTED_OBSTACLE_DISTANCE 50.0 //cm 

#define SMS_TARGET "+8801763106265"
#define EMERGENCY_STRING "EMGNCY"

// class
RH_ASK radioRev(radioSpeed, radioRcvPin, radioRcvPin, 0);
SoftwareSerial GSMSerial(GSM_RX_PIN, GSM_TX_PIN);
SoftwareSerial GPSSerial(GPS_RX_PIN, GPS_TX_PIN);
TinyGsm gsm(GSMSerial);
TinyGPSPlus gps;
HCSR04 ObstacleDistance(SONAR_TRIG_PIN, SONAR_ECHO_PIN);



// variables
uint8_t radioBuff[RADIO_MAX_MSG_LEN];
uint8_t radioBuffLen = sizeof(radioBuff);
String radioData = "";
String commandData = "";
long long timeStamp = 0;

// functions 
String readRadioCommand();
String getLocationData();
void sendSMS(String);
String getSplitValue(String, char, int);
void go(int, int);
void mainProgram();
void test();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  GSMSerial.begin(GSM_SERIAL_BAUD);
  GPSSerial.begin(GPS_SERIAL_BAUD);
  delay(6000);


  // initialization
  Serial.println("Init gsm .");
  // gsm.restart();

  // String gsmInfo = gsm.getModemInfo();
  // Serial.print("gsm info: ");
  // Serial.println(gsmInfo);

  // Serial.println("waiting for network");
  // while (!gsm.waitForNetwork()){
  //   Serial.print(".");
  //   delay(2000);
  // }

  if (!radioRev.init())
  {
    Serial.println("radio init failed");
  }

  Serial.println("starting");

  pinMode(RIGHT_MOTOR_1, OUTPUT);
  pinMode(RIGHT_MOTOR_2, OUTPUT);
  pinMode(RIGHT_MOTOR_SPEED, OUTPUT);
  pinMode(LEFT_MOTOR_1, OUTPUT);
  pinMode(LEFT_MOTOR_2, OUTPUT);
  pinMode(LEFT_MOTOR_SPEED, OUTPUT);


}

void loop() {
  test();
  // mainProgram();
  
}

void test(){

}

void mainProgram(){
  commandData = readRadioCommand();
  if (commandData.indexOf(EMERGENCY_STRING) != -1)
  {
    // do emergency button works
    sendSMS(getLocationData());
  }
  else
  {
    // locomotion work
    // data format "x=255;y=255"  x = formward, y means turn (positive = right turn, negative = left turn)
    if (millis() - timeStamp >= MOTION_DELAY)
    {
      // extract data
      String xData = getSplitValue(commandData, ';', 0);
      String yData = getSplitValue(commandData, ';', 1);
      int forwardMotion = getSplitValue(xData, '=', 1).toInt();
      int turnMotion = getSplitValue(yData, '=', 1).toInt();

      // accept tolerence
      if (forwardMotion < FORWARD_COMMAND_SPEED_MIN)
        forwardMotion = 0;
      if (turnMotion < TURN_COMMAND_SPEED_MIN)
        turnMotion = 0;

      int leftMotorSpeed = forwardMotion + turnMotion;
      int rightMotorSpeed = forwardMotion - turnMotion;

      go(leftMotorSpeed, rightMotorSpeed);
    }
  }
  float obstacleDistance = ObstacleDistance.dist();
  if (obstacleDistance <= MAX_ACCEPTED_OBSTACLE_DISTANCE)
  {
    go(0, 0);
  }
}



String readRadioCommand (){
  // put your main code here, to run repeatedly:
  if (radioRev.recv(radioBuff, &radioBuffLen))
  {
    Serial.println("recv");
    int i;
    radioRev.printBuffer("Got:", radioBuff, radioBuffLen);
    radioData = "";
    for (int j = 0; j < sizeof(radioBuff); j++)
    {
      radioData += (char)radioBuff[j];
    }
    Serial.println(radioData);
    Serial.println(sizeof(radioBuff));
    return String(radioData);
  }
  return String("");
}

void sendSMS(String location){
  String msg = "Emergency!!! Danger in " + location;
  gsm.sendSMS(SMS_TARGET,msg);
}

String getLocationData(){
  GPSSerial.listen();
  if(GPSSerial.available()>0){
    Serial.print(GPSSerial.read());
    if(gps.encode(GPSSerial.read())){
      Serial.print(F("Location: "));
      if (gps.location.isValid())
      {
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(gps.location.lng(), 6);
        return String(gps.location.lat(),6)+String(",")+String(gps.location.lng(),6);
      }
    }
    return "";
  }
  return "";
}

String getSplitValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void go(int leftSpeed, int rightSpeed){
  bool leftForward = true;
  bool rightForward = true;
  if(leftSpeed < 0) {
    leftSpeed *= -1;
    leftForward = false;
  }
  if (rightSpeed <0){
    rightSpeed *= -1;
    rightForward = false;
  }

  analogWrite(LEFT_MOTOR_SPEED, leftSpeed);
  digitalWrite(LEFT_MOTOR_1, leftForward);
  digitalWrite(LEFT_MOTOR_2, !leftForward);
  
  analogWrite(RIGHT_MOTOR_SPEED, rightSpeed);
  digitalWrite(RIGHT_MOTOR_1, rightForward);
  digitalWrite(RIGHT_MOTOR_2, !rightForward);

}