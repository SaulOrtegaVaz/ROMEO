#ifndef DEVICE_APP
#warning No se genera la aplicacion del dispositivo, define DEVICE_APP en platformio.ini para generarla
#else

/*
    Main del cliente, modulo de comunicacion o comunicacion
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "client.hh"
#include "device.hh"

// Definicion de elements conectados al modulo de comunicacion
ROMEODevice device("com", "D12D78");

// Protocolo en cliente (comunicacion)
struct ClientProto { // Recibe los mensajes
    static void intro(WiFiClient& client) {
        char buf[] = "\r\n";
        client.write(buf, 2);
        Serial.println("intro");
    }

    static void run(WiFiClient& client) {
        char cmdline[128]; // Mensaje recibido
        size_t n = client.readBytesUntil('\n', cmdline, sizeof(cmdline));
        cmdline[n++] = '\r';
        cmdline[n++] = '\n';
        cmdline[n] = '\0';
        Serial.println("run");
        Serial.print(cmdline);
        device.runCmd(client, cmdline, n); // Activación del protocolo
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