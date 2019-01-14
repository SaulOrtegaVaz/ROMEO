#ifndef SERVER_HH
#define SERVER_HH
/*
Implementa la máquina de estados del servidor. 
Implementa un AP y servidor en puerto 80. 
Delega la comunicacion en un protocolo.
*/

#include <ESP8266WiFi.h>
#include "led.hh"

#define SSID "Control"
#define PASSWORD "12345678"
#define PORT 80

template <class Protocol>
class ROMEOServer {
public:
    ROMEOServer() : _state(State::MissingAP), _server(80) { pinMode(2, OUTPUT); }

    void run() {
        switch(_state) {
        case State::MissingAP:
            _led.on();
            if (WiFi.softAP(SSID, PASSWORD)) // Activa AP
                _state = State::SoftAP;
            break;
        case State::SoftAP:
            _led.blink(50);
            _server.begin(PORT); // Activa servidor
            _server.setNoDelay(true);
            _state = State::Listening;
            break;
        case State::Listening:
            _led.off();
            checkNewClient();
            runClients();
            break;
        }
    }

    void write(char* buf, size_t n){ // Escritura de datos a todos los clientes
        for(WiFiClient& client: _clients){
            client.write(buf, n);
        }
    }

private:
    void checkNewClient() { // Comprueba si hay nuevos clientes
        WiFiClient client = _server.available();
        if (client) addNewClient(client);
    }

    void runClients() { // Aplica el protocolo en todos los clientes del registro
        for (WiFiClient& client: _clients){
            if (client && client.available()) {
                _led.off();
                Protocol::run(client, _server);
            }
        }
    }

    bool addNewClient(WiFiClient& newClient) { // Añade el nuevo cliente al registro
        newClient.setNoDelay(true);
        _led.on();
        Protocol::intro(newClient, _server);
        for (WiFiClient& client: _clients){
            if (!client) {
                client = newClient;
                return true;
            }
        }
        return false;
    }

private:
    enum class State {
        MissingAP,
        SoftAP,
        Listening,
    };

    WiFiServer _server;
    WiFiClient _clients[4]; // Registro de clientes del servidor
    State _state;
    LEDControl _led;
};

#endif