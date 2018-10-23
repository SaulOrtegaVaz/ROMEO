#ifndef CONTROL_APP
#warning No se genera la aplicacion del controlador, define CONTROL_APP en platformio.ini para generarla
#else

/*
    Main del servidor, modulo de control o control
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "server.hh"
#include "device.hh"

// Definicion de elements conectados al modulo de control
ROMEODevice device("control", "M12");

// Protocolo en servidor (control)
struct ControlProto { // Reenvia a todos los clientes las ordenes (Relays commands to clients)
    /*static void run(WiFiClient& client, WiFiServer& server) {
        char cmdline[128]; // Mensaje recibido
        size_t n = client.readBytesUntil('\n', cmdline, sizeof(cmdline));
        Serial.print("Comando recibido: ");
        Serial.println(cmdline);
        if (n > 2) // Si mensaje no está vacio
            module.write(cmdline, n); // Envio a todos los clientes (Send to all)
        //device.runCmd(client, cmdline, n); // Activación del protocolo
    }*/
    static void run(WiFiClient& client, WiFiServer& server);
};

using ROMEOModule = ROMEOServer<ControlProto>;
ROMEOModule module;

void ControlProto::run(WiFiClient& client, WiFiServer& server) {
    char cmdline[128]; // Mensaje recibido
    size_t n = client.readBytesUntil('\n', cmdline, sizeof(cmdline));
    if (n > 2) { // Si mensaje no está vacio
        module.write(cmdline, n); // Envio a todos los clientes (Send to all)
        module.write("\r\n", 2);
    }
    device.runCmd(client, cmdline, n); // Activación del protocolo
}

void setup() {
    Serial.begin(115200);
}

void loop() {
    module.run();
}

#endif