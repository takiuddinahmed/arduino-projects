// headers
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif
#include <NewPing.h>

// define
#define radioSpeed 2000
#define radioRcvPin 2
#define RADIO_MAX_MSG_LEN 12
#define NODEMCU_RX_PIN 10
#define NODEMCU_TX_PIN 9
#define GPS_RX_PIN 11
#define GPS_TX_PIN 12
#define DEBUG_SERIAL_BAUD 9600
#define NODEMCU_SERIAL_BAUD 9600
#define GPS_SERIAL_BAUD 4800

#define LEFT_MOTOR_1 A3
#define LEFT_MOTOR_2 A4
#define LEFT_MOTOR_SPEED 6
#define RIGHT_MOTOR_1 A0
#define RIGHT_MOTOR_2 A1
#define RIGHT_MOTOR_SPEED 5
#define FORWARD_COMMAND_SPEED_MIN 50
#define TURN_COMMAND_SPEED_MIN 50
#define MOTION_DELAY 100

#define SONAR_TRIG_PIN 8
#define SONAR_ECHO_PIN 7
#define MAX_ACCEPTED_OBSTACLE_DISTANCE 30.0 //cm

#define EMERGENCY_STRING "EMERGENCY"

// class
RH_ASK radioRev(radioSpeed, radioRcvPin, radioRcvPin, 0);
SoftwareSerial NodeMcu(NODEMCU_RX_PIN, NODEMCU_TX_PIN);
// SoftwareSerial GPSSerial(GPS_RX_PIN, GPS_TX_PIN);
// TinyGPSPlus gps;
NewPing ObstacleDistance(SONAR_TRIG_PIN, SONAR_ECHO_PIN);

// variables
uint8_t radioBuff[RADIO_MAX_MSG_LEN];
uint8_t radioBuffLen = sizeof(radioBuff);
String radioData = "";
String commandData = "";
long long timeStamp = 0;
bool forwardCommand = false;

// functions
String readRadioCommand();
String getLocationData();
void sendSMS(String);
String getSplitValue(String, char, int);
void go(int, int);
void mainProgram();
void test();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  NodeMcu.begin(NODEMCU_SERIAL_BAUD);

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
  // go(100,100);
  // sendSMS("a critical place");
}

void loop()
{
  test();
  // mainProgram();
  // go(200, 200);
  // Serial.println("Here");

  // commandData = readRadioCommand();
  // if(commandData.length()){
  //   Serial.println(commandData);  
  // }

  // float obstacleDistance = (float) ObstacleDistance.ping_cm();
  // Serial.println(obstacleDistance);
  // delay(1000);
}

void test()
{
  commandData = readRadioCommand();
  // Serial.println(commandData);
  if (commandData.length())
  {
    if (commandData.indexOf(EMERGENCY_STRING) != -1)
    {
      // do emergency button works
      Serial.println("EMMMMEEE");
      // sendSMS(getLocationData());
      NodeMcu.println(EMERGENCY_STRING);
    }
    else
    {
      // locomotion work
      // data format "x=255;y=255"  x = formward, y means turn (positive = right turn, negative = left turn)
      // if (millis() - timeStamp >= MOTION_DELAY)
      // {
      Serial.println(commandData);
      // extract data
      String xData = getSplitValue(commandData, ';', 0);
      String yData = getSplitValue(commandData, ';', 1);
      int forwardMotion = getSplitValue(xData, '=', 1).toInt();
      int turnMotion = getSplitValue(yData, '=', 1).toInt();

      // accept tolerence
      if (abs(forwardMotion) < FORWARD_COMMAND_SPEED_MIN)
        forwardMotion = 0;
      if (abs(turnMotion) < TURN_COMMAND_SPEED_MIN)
        turnMotion = 0;

      int leftMotorSpeed = forwardMotion - turnMotion;
      int rightMotorSpeed = forwardMotion + turnMotion;
      Serial.print(leftMotorSpeed);
      Serial.print("   ");
      Serial.println(rightMotorSpeed);

      if(leftMotorSpeed > 0 && rightMotorSpeed >0){
        forwardCommand = true;
      }
      else forwardCommand = false;

      go(leftMotorSpeed, rightMotorSpeed);
      // timeStamp = millis();
      // }
    }
    
    // Serial.println(obstacleDistance);
    if (forwardCommand )
    {
      float obstacleDistance = ObstacleDistance.ping_cm();
      if (obstacleDistance <= MAX_ACCEPTED_OBSTACLE_DISTANCE && obstacleDistance >0)
          go(0, 0);
    }
  }
}

void mainProgram()
{
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

      int leftMotorSpeed = forwardMotion - turnMotion;
      int rightMotorSpeed = forwardMotion + turnMotion;

      go(leftMotorSpeed, rightMotorSpeed);
    }
  }
  float obstacleDistance = ObstacleDistance.ping_median();
  if (obstacleDistance <= MAX_ACCEPTED_OBSTACLE_DISTANCE)
    {
      go(0, 0);
    }
}

String readRadioCommand()
{
  // put your main code here, to run repeatedly:
  if (radioRev.recv(radioBuff, &radioBuffLen))
  {
    // Serial.println("recv");
    int i;
    // radioRev.printBuffer("Got:", radioBuff, radioBuffLen);
    radioData = "";
    for (int j = 0; j < sizeof(radioBuff); j++)
    {
      radioData += (char)radioBuff[j];
    }
    // Serial.println(radioData);
    // Serial.println(sizeof(radioBuff));
    return String(radioData);
  }
  return String("");
}

// void sendSMS(String location)
// {
//   String msg = "Emergency!!! Danger in " + location;
//   gsm.sendSMS(SMS_TARGET, msg);
// }

// String getLocationData(){
//   GPSSerial.listen();
//   if(GPSSerial.available()>0){
//     Serial.print(GPSSerial.read());
//     if(gps.encode(GPSSerial.read())){
//       Serial.print(F("Location: "));
//       if (gps.location.isValid())
//       {
//         Serial.print(gps.location.lat(), 6);
//         Serial.print(F(","));
//         Serial.print(gps.location.lng(), 6);
//         return String(gps.location.lat(),6)+String(",")+String(gps.location.lng(),6);
//       }
//     }
//     return "";
//   }
//   return "";
// }

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

void go(int leftSpeed, int rightSpeed)
{
  bool leftForward = true;
  bool rightForward = true;
  if (leftSpeed < 0)
    {
      leftSpeed *= -1;
      leftForward = false;
    } 
  if (rightSpeed < 0)
    {
      rightSpeed *= -1;
      rightForward = false;
    }
  

  if(leftSpeed > 255) leftSpeed = 255;
  if (rightSpeed > 255) rightSpeed = 255;

  analogWrite(LEFT_MOTOR_SPEED, leftSpeed);
  digitalWrite(LEFT_MOTOR_1, leftForward);
  digitalWrite(LEFT_MOTOR_2, !leftForward);

  analogWrite(RIGHT_MOTOR_SPEED, rightSpeed);
  digitalWrite(RIGHT_MOTOR_1, rightForward);
  digitalWrite(RIGHT_MOTOR_2, !rightForward);
}