#ifndef CONTROL_APP
#warning No se genera la aplicación del controlador, define CONTROL_APP en platformio.ini para generarla
#else

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "server.hh"
#include "device.hh"

ROMEODevice device("ctrl", "M45");

struct ControlProto { // Relays commands to clients
    static void run(WiFiClient& client, WiFiServer& server) {
        char cmdline[128];
        size_t n = client.readBytesUntil('\n', cmdline, sizeof(cmdline));
        if (n > 2) // message not empty
            server.write(cmdline, n); // Send to all
        device.runCmd(client, cmdline, n);
    }
};

using ROMEOModule = ROMEOServer<ControlProto>;
ROMEOModule module;

void setup() {
    Serial.begin(115200);
}

void loop() {
    module.run();
}

#endif