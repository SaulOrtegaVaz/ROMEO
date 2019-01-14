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
        client.write(buf, 2)
    }

    static void run(WiFiClient& client) {
        char cmdline[128]; // Mensaje recibido
        size_t n = client.readBytesUntil('\n', cmdline, sizeof(cmdline));
        cmdline[n] = '\0';
        device.runCmd(client, cmdline, n); // Activación del protocolo
    }
};

using ROMEOModule = ROMEOClient<ClientProto>;
ROMEOModule module;

void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
}

void loop() {
    module.run();
}

#endif