#ifndef CLIENT_HH
#define CLIENT_HH
/*
Implementa la máquina de estados de los clientes. 
Mantiene una conexión persistente. 
Delega la comunicacion en un protocolo.
*/

#include <ESP8266WiFi.h>

template <class Protocol>
class ROMEOClient {
public:
    ROMEOClient() : _state(State::Disconnected) {}

    void run() {
        switch(_state) {
        case State::Disconnected:
            WiFi.mode(WIFI_STA); // Activa el modo station Wi-Fi
            WiFi.begin("Control", "12345678"); // Se conecta a la red Wi-Fi señalada
            _state = State::Waiting;
            break;
        case State::Waiting:
            if (WiFi.status() == WL_CONNECTED) { // Si está conectado a la red Wi-Fi
                _state = State::Associated;
            }
            break;
        case State::Associated:
            if (checkAssociated()) { // Si permanece conectado a la red Wi-Fi
                digitalWrite(2, HIGH); // Led On <-> Asociado a AP
                _client.connect(IPAddress(192,168,4,1), 80);
                if (_client) // Si está conectado al servidor
                    _client.setNoDelay(true);
                    _state = State::Connected;
            }
            break;
        case State::Connected:
            // Si está asociado a la red Wi-Fi, si está conectado al servidor y si hay datos disponibles
            if (checkAssociated() && checkConnected() && _client.available()) {
                Protocol::run(_client); // Aplica protocolo
            }
            break;
        }
    }

private:
    bool checkAssociated() { // Comprueba conexión a red Wi-Fi
        if (WiFi.status() == WL_CONNECTED) return true;
        digitalWrite(2, LOW); // Led Off <-> Desconectado del AP
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

    State _state;
    WiFiClient _client;
};

#endif