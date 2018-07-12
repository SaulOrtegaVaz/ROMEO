#ifndef CLIENT_HH
#define CLIENT_HH

#include <ESP8266WiFi.h>

template <class Protocol>
class ROMEOClient {
public:
    void run() {
        switch(_state) {
        case State::Disconnected:
            WiFi.mode(WIFI_STA);
            WiFi.begin("Control", "12345678");
            _state = State::Waiting;
            break;
        case State::Waiting:
            if (WiFi.status() == WL_CONNECTED)
                _state = State::Associated;
            break;
        case State::Associated:
            if (checkAssociated()) {
                _client.connect(IPAddress(192,168,4,1), 80);
                if (_client) 
                    _state = State::Connected;
            }
            break;
        case State::Connected:
            if (checkAssociated() && checkConnected() && _client.available())
                Protocol::run(_client);
            break;
        }
    }

private:
    bool checkAssociated() {
        if (WiFi.status() == WL_CONNECTED) 
            return true;
        _state = State::Disconnected;
        return false;
    }

    bool checkConnected() {
        if (_client) return true;
        _state = State::Associated;
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