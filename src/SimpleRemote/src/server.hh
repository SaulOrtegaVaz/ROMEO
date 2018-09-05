#ifndef SERVER_HH
#define SERVER_HH

#include <ESP8266WiFi.h>

template <class Protocol>
class ROMEOServer {
public:
    ROMEOServer() : _state(State::MissingAP), _server(80) {}

    void run() {
        switch(_state) {
        case State::MissingAP:
            if (WiFi.softAP("Control", "12345678")) _state = State::SoftAP;
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
        if (client) addNewClient(client);
    }

    bool addNewClient(const WiFiClient& newClient) {
        for (WiFiClient& client: _clients)
            if (!client) {
                client = newClient;
                return true;
            }
        return false;
    }

    void runClients() {
        for (WiFiClient& client: _clients){
            if (client) Protocol::run(client, _server);
        }
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