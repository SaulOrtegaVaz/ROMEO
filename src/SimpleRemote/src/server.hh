#ifndef SERVER_HH
#define SERVER_HH
/*
Implementa la máquina de estados del servidor. 
Implementa un AP y servidor en puerto 80. 
Delega la comunicacion en un protocolo.
*/

#include <ESP8266WiFi.h>

template<class Protocol>
class ROMEOServer {
public:
    ROMEOServer() : _state(State::MissingAP), _server(80) {}

    void run() {
        switch(_state) {
        case State::MissingAP:
            if (WiFi.softAP("Control", "12345678"))
                _state = State::SoftAP;
            break;
        case State::SoftAP:
            _server.begin();
            _state = State::Listening;
            break;
        case State::Listening:
            checkNewClient();
            runClients();
            break;
        }
    }

private:
    void checkNewClient() {
        WiFiClient client = _server.available();
        if (client)
            addNewClient(client);
    }

    void runClients() {
        for (WiFiClient& client: _clients)
            if (client && client.available()) 
                Protocol::run(client, _server);
    }

    bool addNewClient(const WiFiClient& newClient) {
        for (WiFiClient& client: _clients)
            if (!client) {
                client = newClient;
                return true;
            }
        return false;
    }

private:
    enum class State {
        MissingAP,
        SoftAP,
        Listening,
    };

    State _state;
    WiFiServer _server;
    WiFiClient _clients[5];
};

#endif