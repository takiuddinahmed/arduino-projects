#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>

// definations
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 4
#define POWER_MONITORING_DELAY 1000
#define MTE_DISPLAY_DELAY 2000
#define POWER_DISPLAY_DELAY 10000

// sensor pin config
int top_left_sensor_pin = A2;
int top_right_sonsor_pin = A3;
int bottom_left_sensor_pin = A1;
int bottom_right_sensor_pin = A0;

// motor pin config
int base_servo_pin = 7;
int top_servo_pin = 8;

// config
int base_rotate_left = 100;
int base_rotate_right = 75;
int base_rotate_stop = 93;
int top_rotate_unit = 1;
int threshold = 300;
int max_acceptable_diff = 120;
int control_delay = 400;
long long control_time_stamp = 0;
long long power_monitoring_time_stamp = 0;
long long mte_display_time_stamp = 0;
bool mte_display_state = true;

//  classes
Servo base_servo;
Servo top_servo;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_INA219 ina219;


// functions
void control(int, int, int, int);
void updateDisplay(float, float);
void base_servo_change(int);
void top_servo_change(int);
void updateMteDisplay();

void setup()
{
  Serial.begin(9600);
  base_servo.attach(base_servo_pin);
  top_servo.attach(top_servo_pin);
  top_servo.write(50);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  ina219.begin();
  updateMteDisplay();

}

void loop()
{
  if(millis() - control_time_stamp >= control_delay){
    // read sensor value
    int top_left_sensor = analogRead(top_left_sensor_pin);
    int top_right_sensor = analogRead(top_right_sonsor_pin);
    int bottom_left_sensor = analogRead(bottom_left_sensor_pin);
    int bottom_right_sensor = analogRead(bottom_right_sensor_pin);
    Serial.print(top_left_sensor);
    Serial.print("   ");
    Serial.print(top_right_sensor);
    Serial.print("   ");
    Serial.print(bottom_left_sensor);
    Serial.print("   ");
    Serial.println(bottom_right_sensor);
    // control
    control(top_left_sensor, top_right_sensor, bottom_left_sensor, bottom_right_sensor);
    control_time_stamp = millis();
  }
  
  if (!mte_display_state && millis() - power_monitoring_time_stamp >= POWER_MONITORING_DELAY)
  {
    // read sensor value
    float shunt_voltage = ina219.getShuntVoltage_mV();  //mv
    float bus_voltage = ina219.getBusVoltage_V();  //v
    float current = ina219.getCurrent_mA();   // mA
    float voltage = bus_voltage + (shunt_voltage / 1000.0);
    voltage = voltage>0 ? voltage : 0;
    current = current>0 ? current :  (voltage> 0 ? current*-1 : 0);
    updateDisplay(voltage, current);

    //display
    power_monitoring_time_stamp = millis();
  }
  if(!mte_display_state && millis() - mte_display_time_stamp >= POWER_DISPLAY_DELAY){
    updateMteDisplay();
    mte_display_state = true;
    mte_display_time_stamp = millis();
  }
  if(mte_display_state && millis() - mte_display_time_stamp >= MTE_DISPLAY_DELAY){
    mte_display_state = false;
    mte_display_time_stamp = millis();
  }

}

void control(int top_left_sensor, int top_right_sensor, int bottom_left_sensor, int bottom_right_sensor)
{

  // all dark (night)
  // if (top_right_sensor<threshold & top_left_sensor<threshold & bottom_left_sensor<threshold & bottom_right_sensor<threshold){
  //   // do nothing
  //   // delay(100);
  // }
  // first check base
  if ((top_left_sensor - top_right_sensor > max_acceptable_diff))
  {
    base_servo_change(base_rotate_right);
  }

  else if (top_right_sensor - top_left_sensor > max_acceptable_diff)
  {
    base_servo_change(base_rotate_left);
  }
  else if ((bottom_left_sensor - bottom_right_sensor > max_acceptable_diff))
  {
    base_servo_change(base_rotate_right);
  }

  else if (bottom_right_sensor - bottom_left_sensor > max_acceptable_diff)
  {
    base_servo_change(base_rotate_left);
  }

  else if (top_left_sensor - bottom_left_sensor > max_acceptable_diff)
  {
    top_servo_change(top_rotate_unit);
  }

  else if (bottom_left_sensor - top_left_sensor > max_acceptable_diff)
  {
    top_servo_change(-1 * top_rotate_unit);
  }

  else if (top_right_sensor - bottom_right_sensor > max_acceptable_diff)
  {
    top_servo_change(top_rotate_unit);
  }

  else if (bottom_right_sensor - top_right_sensor > max_acceptable_diff)
  {
    top_servo_change(-1 * top_rotate_unit);
  }
}

void base_servo_change(int deg)
{
  Serial.print("Base Servo Change : ");
  // Serial.print(base_servo.read());
  // Serial.print(" to ");
  Serial.println(deg);
  // base_servo.write(base_servo.read() + deg);
  base_servo.write(deg);
  delay(200);
  base_servo.write(base_rotate_stop);
}

void top_servo_change(int deg)
{
  Serial.print("top Servo Change : ");
  Serial.print(top_servo.read());
  Serial.print(" to ");
  Serial.println(deg);
  top_servo.write(top_servo.read() + deg);
}

void updateDisplay(float voltage, float current){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("  Power");
  // display.setCursor(1,1);
  display.print("V: ");
  display.print(voltage, 1);
  display.println(" v");
  // display.setCursor(2,2);
  display.print("I: ");
  display.print(current, 0);
  display.println(" mA");
  // display.setCursor(3,3);
  display.print("P: ");
  display.print(voltage * current,0);
  display.println(" mW");

  display.display();
}

void updateMteDisplay(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0,0);
  display.println();
  display.print("  MTE");
  display.display();
}
