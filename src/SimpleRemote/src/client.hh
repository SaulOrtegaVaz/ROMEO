#ifndef CLIENT_HH
#define CLIENT_HH
/*
Implementa la máquina de estados de los clientes. 
Mantiene una conexión persistente. 
Delega la comunicacion en un protocolo.
*/

#include <ESP8266WiFi.h>
#include "led.hh"

#define SSID "Control"
#define PASSWORD "12345678"
#define IPADDR IPAddress(192,168,4,1)
#define PORT 80

template <class Protocol>
class ROMEOClient {
public:
    ROMEOClient() : _state(State::Disconnected) { }


    void run() {
        switch(_state) {
        case State::Disconnected:
            _led.on();
            WiFi.mode(WIFI_STA); // Activa el modo station Wi-Fi
            WiFi.begin(SSID, PASSWORD); // Se conecta a la red Wi-Fi señalada
            _state = State::Waiting;
            break;
        case State::Waiting:
            _led.blink(50);
            if (WiFi.status() == WL_CONNECTED) { // Si está conectado a la red Wi-Fi
                _state = State::Associated;
            }
            break;
        case State::Associated:
            _led.blink(150);
            if (checkAssociated()) { // Si permanece conectado a la red Wi-Fi
                _client.connect(IPADDR, PORT);
                if (_client) {// Si está conectado al servidor
                    _led.off();
                    _client.setNoDelay(true);
                    _state = State::Connected;
                    Protocol::intro(_client);
                }
            }
            break;
        case State::Connected:
            // Si está asociado a la red Wi-Fi, si está conectado al servidor y si hay datos disponibles
            if (checkConnected() && _client.available()) {
                    Protocol::run(_client); // Aplica protocolo
            }
            break;
        }
    }

private:
    bool checkAssociated() { // Comprueba conexión a red Wi-Fi
        if (WiFi.status() == WL_CONNECTED) return true;
        _state = State::Disconnected; // Si no, vuelve a estado disconnected
        return false;
    }

    bool checkConnected() { // Comprueba conexión al servidor
        if (_client) return true;
        _state = State::Associated; // Si no, vuelve a estado associated
        return false;
    }

private:
    enum class State {
        Disconnected,
        Waiting,
        Associated,
        Connected,
    };

    WiFiClient _client;
    State _state;
    LEDControl _led;
};

#endif