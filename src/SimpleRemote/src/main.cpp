#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "server.hh"
#include "client.hh"

struct SimpleProto {
    static void run(WiFiClient& client) {
        if (!client.available()) return;
        String cmd = client.readStringUntil('\n');
        Serial.println(cmd);
    }
};

using ROMEOModule = ROMEOClient<SimpleProto>;
//using ROMEOModule = ROMEOServer<SimpleProto>;

ROMEOModule module;

void setup() {
    Serial.begin(115200);
}

void loop() {
    module.run();
}