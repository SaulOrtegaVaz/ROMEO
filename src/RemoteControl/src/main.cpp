#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "control-ap.hh"
#include "control-estacion.hh"
#include "general.hh"

void setup() {
    Serial.begin(115200);
    servidor.begin();
    estcontrol.conexionRedex("","");
}

int sensores = 0;

void loop() {
    if(WiFi.isConnected()){
        Serial.println(WiFi.localIP());
        estcontrol.setSensor();
        estcontrol.webControl();
    }
    delay(2000);
}
