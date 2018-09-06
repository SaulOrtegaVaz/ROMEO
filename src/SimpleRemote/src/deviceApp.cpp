#include <Arduino.h>
#ifndef DEVICE_APP
#warning No se genera la aplicación del dispositivo, define DEVICE_APP en platformio.ini para generarla
#else

#include <ESP8266WiFi.h>
#include "server.hh"
#include "client.hh"
#include "device.hh"

ROMEODevice device("com", "M.D.");

struct ClientProto { // Parse command, dispatch to device
    static void run(WiFiClient& client) {
        char cmdline[128];
        char *p;
        size_t n = client.readBytesUntil('\n', cmdline, sizeof(cmdline));
        if (n < 1) return;
        const char* argv[4] = { nullptr };
        for (uint8_t i = 0; i < 4; ++i) {
            argv[i] = strtok_r(i? nullptr: cmdline, " \t\r\n", &p);
            if (argv[i] == nullptr) break;
        }
        device.runCmd(client, argv);
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