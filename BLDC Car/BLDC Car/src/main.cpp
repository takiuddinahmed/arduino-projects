#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Servo.h>
// #include <ESC.h>

// definations
#define ble_rx_pin 8
#define ble_tx_pin 9
#define bldc_left_pin 11
#define bldc_right_pin 12
#define ble_baudrate 9600
#define LEFT_MIN_SPEED 1240
#define RIGHT_MIN_SPEED 1340
#define STOP_SPEED 1000

// classes
SoftwareSerial ble(ble_rx_pin, ble_tx_pin);
// ESC bldc_left;
// ESC bldc_right;
Servo bldc_left;
Servo bldc_right;


// functions
void forward();
void left();
void right();
void stp();



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  ble.begin(ble_baudrate);

  bldc_left.attach(bldc_left_pin,1000,2000);
  bldc_right.attach(bldc_right_pin,1000,2000);
  
  // bldc_left.writeMicroseconds(1240);
  bldc_left.writeMicroseconds(1000);
  bldc_right.writeMicroseconds(1000);
  delay(1000);
  Serial.println("Start");


  // testing
  // bldc_left.writeMicroseconds(LEFT_MIN_SPEED + 5);
  // bldc_right.writeMicroseconds(RIGHT_MIN_SPEED + 20);
}

void loop() {
  if(ble.available()){
    char a = ble.read();
    switch(a){
      case 'f':
          Serial.println("Forward");
          forward();
          break;
      case 'l':
          Serial.println("Left");
          left();
          break;
      case 'r':
          Serial.println("Right");
          right();
          break;
      case 's':
          Serial.println("Stop");
          stp();
          break;
      default:
          Serial.println("Default");
          Serial.println(a);
          stp();
          break;

    }
  }

}


void forward(){
  bldc_left.writeMicroseconds(LEFT_MIN_SPEED + 5);
  bldc_right.writeMicroseconds(RIGHT_MIN_SPEED + 20);
}

void stp(){
  bldc_left.writeMicroseconds(STOP_SPEED);
  bldc_right.writeMicroseconds(STOP_SPEED);
}

void right(){
  bldc_left.writeMicroseconds(LEFT_MIN_SPEED + 60);
  bldc_right.writeMicroseconds(STOP_SPEED);
}

void left(){
  bldc_right.writeMicroseconds(RIGHT_MIN_SPEED + 150);
  bldc_left.writeMicroseconds(STOP_SPEED);
}

