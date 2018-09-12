#include <Arduino.h>
#ifndef DEVICE_APP
#warning No se genera la aplicación del dispositivo, define DEVICE_APP en platformio.ini para generarla
#else

#include <ESP8266WiFi.h>
#include "server.hh"
#include "client.hh"
#include "device.hh"

ROMEODevice device("com", "D34D68");

struct ClientProto { // Parse command, dispatch to device
    static void run(WiFiClient& client) {
        char cmdline[128];
        size_t n = client.readBytesUntil('\n', cmdline, sizeof(cmdline));
        device.runCmd(client, cmdline, n);
    }
};

using ROMEOModule = ROMEOClient<ClientProto>;
ROMEOModule module;

void setup() {
    Serial.begin(115200);
}

void loop() {
    module.run();
}

#endif