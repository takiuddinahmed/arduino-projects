#include <Arduino.h>
#include <LiquidCrystal.h>
#include <HX711.h>
#include <NewPing.h>

// pin def
#define SONAR_TRIG_PIN 13
#define SONAR_ECHO_PIN 12
#define LED_PIN_1 2
#define LED_PIN_2 3
#define LOAD_DOUT_PIN 10
#define LOAD_SCK_PIN 11
#define BUTTON_UP_PIN A2
#define BUTTON_STOP_PIN A3
#define BUTTON_DOWN_PIN A1

// defines
#define CALIBRATION_FACTOR 17555.0 //150555.0 // -7050.0 //150555.5
#define DISPLAY_DELAY 300
#define SERIAL_UPLOAD_DELAY 300
#define MAX_ACCEPTABLE_DISTANCE 70
#define MIN_ACCEPTABLE_DISTANCE 5
#define LOAD_THRESHOLD 0.5

// classes
LiquidCrystal lcd(4,5,6,7,8,9);
HX711 scale;
NewPing sonar(SONAR_TRIG_PIN,SONAR_ECHO_PIN,200);


// variables
unsigned long displayTimestamp = 0;
unsigned long serialTimestamp = 0;
char command = 's';    // 's' --> stop, 'u' ---> up, 'd' ---> Down 
float load = 0.0;
float prevLoad = 0.0;
float irDistance = 0.0;
float irDistanceInit = 0.0;
float deflection = 0.0;
float distance = 0.0;
float prevDistance = 200.0;


// functions
void checkBtnClick();
void displayData();
float measureDeflection();
void updateSensorVal();
void display();

void setup() {
  
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("  Loading.....  ");
  Serial.begin(9600);
  
  
  // pinmode
  pinMode(LED_PIN_1,OUTPUT);
  pinMode(LED_PIN_2,OUTPUT);
  pinMode(BUTTON_UP_PIN,INPUT_PULLUP);
  pinMode(BUTTON_STOP_PIN,INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN,INPUT_PULLUP);

  scale.begin(LOAD_DOUT_PIN,LOAD_SCK_PIN);
  scale.get_units(10);

  delay(100);


  float s = 150555.0;
  // scale.set_scale(s);
  // long zeropoint = scale.read_average();
  // scale.set_scale(CALIBRATION_FACTOR);
  // scale.tare();
  // scale.get_units(10);
  delay(500);
  lcd.clear();

  scale.set_scale();
  scale.tare(10);
  // Serial.println("-----------------------------------------");
  // Serial.println(scale.get_units(10));
  Serial.print("zero point : ");
  // Serial.println(scale.read_average());
  float calibration_factor = 17555.0;   // 268555
  
  scale.set_scale(calibration_factor);
  scale.set_offset(200);

  // while(1){
  //     Serial.print("Reading: ");
  //     Serial.print(scale.get_units(), 1);
  //     Serial.print(" kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  //     Serial.print(" calibration_factor: ");
  //     Serial.print(scale.get_scale());
  //     Serial.println();

  //     if(Serial.available())
  //     {
  //       char temp = Serial.read();
  //       if(temp == '+' || temp == 'a')
  //         calibration_factor += 1000;
  //       else if(temp == '-' || temp == 'z'){
  //         calibration_factor -= 1000;
  //       }
  //       scale.set_scale(calibration_factor);

  //     }
  //     delay(200);
  // }

  prevDistance = sonar.ping_cm();
  // updateSensorVal();
}

void loop() {
  checkBtnClick();
  // if(command == 'd'){
    updateSensorVal();
  // }
  checkBtnClick();
  if(command == 's'){
    digitalWrite(LED_PIN_1,LOW);
    digitalWrite(LED_PIN_2,LOW);
  }
  
  else if(command == 'u'){
    digitalWrite(LED_PIN_1,LOW);
    digitalWrite(LED_PIN_2,HIGH);
  }
  else if(distance > MAX_ACCEPTABLE_DISTANCE || distance < MIN_ACCEPTABLE_DISTANCE){
    // command = 's';
    digitalWrite(LED_PIN_1,LOW);
    digitalWrite(LED_PIN_2,LOW);
  }

  else if(command == 'd'){
    digitalWrite(LED_PIN_1,HIGH);
    digitalWrite(LED_PIN_2,LOW);
  }
  if(millis() - displayTimestamp > DISPLAY_DELAY){
    display();
    displayTimestamp = millis();
  }
  if(millis() - serialTimestamp > SERIAL_UPLOAD_DELAY ){
    Serial.print("START;");
    Serial.print("DISTANCE:");
    Serial.print(deflection);
    Serial.print(";LOAD:");
    Serial.print(load);
    Serial.println(";END");
    
    serialTimestamp = millis();
  }

  delay(50);


}


void checkBtnClick (){
  bool upMove = digitalRead(BUTTON_UP_PIN);
  bool stopMove = digitalRead(BUTTON_STOP_PIN);
  bool downMove = digitalRead(BUTTON_DOWN_PIN);

  if(!stopMove){
    command = 's';
  }
  else if (!downMove) {
    command = 'd';
  }
  else if (!upMove) {
    command = 'u';
  }

}

void updateSensorVal(){
  // float a0 = scale.get_units();
  // float a1= scale.get_units();
  // float a2 = scale.get_units();
  // float new_load = min(a0,min(a1,a2));
  float new_load = scale.get_units() - 5.2;
  new_load /= 8.0;
  irDistance = measureDeflection();
  float new_distance = sonar.ping_cm();

  // Serial.print(a0);
  // Serial.print("   ");
  // Serial.print(a1);
  // Serial.print("   ");
  // Serial.println(a2);
  // Serial.println(new_load);
  
  if (new_load > 0 & new_load < 50) 
  {
    load = new_load;
    prevLoad = load;
    if(load < LOAD_THRESHOLD) load = 0.0;
    if (load > LOAD_THRESHOLD){
      if(!irDistanceInit) irDistanceInit = irDistance;
      deflection = irDistance - irDistanceInit ;
    }
    else deflection = 0;
      
    // Serial.print(irDistance);
    // Serial.print("    ");
    // Serial.print(irDistanceInit);
    // Serial.print("     ");
    // Serial.println(deflection);
  }
  if( distance - prevDistance < 2) {
    prevDistance = distance;
    distance = new_distance;
  }
}


float measureDeflection(){
  float val = 0;
  float d = -1;
  for(int i=0;i<50;i++){
    val += analogRead(A0);
    delay(1);
  }
  val /= 50.0;
  if(val < 195 & val >150){
    d = 20.0+(195-val)/25.0;
  }
  else if(val > 194 & val < 290){
    d = 17.0+ (290-val)/32.0;
  }
  else if(val > 289 & val < 400){
    d = 14.0 + (400-val)/35.0;
  }
    else if(val >397 & val < 500){
    d = 11.0 + (500-val)/40.0;
  }
     else if(val >498 & val < 600){
    d = 8.0 + (600-val)/40.0;
  }
     else if(val >597 & val < 700){
    d = 5.0 + (700-val)/40.0;
  }
     else if(val >697 & val < 800){
    d = 3.0 + (800-val)/40.0;
  }
  return d;
}


void display(){
  lcd.setCursor(0,0);
  lcd.print("Dist. ");
  lcd.print(deflection,3);
  lcd.print("     ");
  lcd.setCursor(0,1);
  lcd.print("Load: ");
  lcd.print(load);
  lcd.print("    ");
}

