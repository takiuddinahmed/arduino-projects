#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <./secret.h>

#define EMERGENCY_STRING "EMERGENCY"

String data = "";

void setup()
{
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    // Serial.print(".");
  }
  // Serial.println("");
  // Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()){
    data = "";
    data = Serial.readString();
      Serial.println(data);
    if (data.indexOf(EMERGENCY_STRING)>-1){
      // Serial.println("yes... emergency");
      if(WiFi.status() == WL_CONNECTED){
        // Serial.println("Http begining");
        HTTPClient http;
        http.begin(SERVER);
        http.addHeader("Content-Type","application/json");
        // Serial.print("Http sending post");
        int httpResponseCode = http.POST(MSG_JSON_STRING);
        // Serial.println(httpResponseCode);
        data = "";
      }
    }

    
  }
}