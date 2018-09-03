#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "server.hh"
#include "client.hh"

struct ClientProto {
    static void run(WiFiClient& client) {
        if (!client.available()) return;
        String cmdline = client.readStringUntil('\n');
        char cmd = cmdline[0];
        char from = cmdline[2];
        char dest = cmdline[4];
        if (dest != id) return;
        Serial.println(cmdline);
    }
    static char id;
};

struct ControlProto {
    static void run(WiFiClient& client, WiFiServer& server) {
        if (!client.available()) return;
        String str = client.readStringUntil('\n');
        if (str.length() > 2)
            server.write(str.c_str(), str.length()); // Send to all
    }
};

/*
W 0 1 3 0.5  Escribe en el dispositivo 3 del módulo 1 un 0.5. Lo pide el módulo 0
R 1 2 0 Lee el dispositivo 0 del módulo 2.  Lo pide el módulo 1
V 2 1 0 0.534  Responde a lectura informando del valor del dispositivo 0 del módulo 2. Se lo envía al 1
*/


// char ClientProto::id = 'L';
// using ROMEOModule = ROMEOClient<ClientProto>;
using ROMEOModule = ROMEOServer<ControlProto>;

ROMEOModule module;

void setup() {
    Serial.begin(115200);
}

void loop() {
    module.run();
}