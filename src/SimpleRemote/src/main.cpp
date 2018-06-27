#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "server.hh"

void setup() {
    Serial.begin(115200);
}

ROMEOServer server;

void loop() {
    server.run();
    delay(2000);
}