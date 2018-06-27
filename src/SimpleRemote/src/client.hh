#ifndef CLIENT_HH
#define CLIENT_HH

#include <ESP8266WiFi.h>

class ROMEOClient {
public:
    ROMEOClient(WiFiClient clnt) : _client(clnt) {}

    bool run() {
        if (!_client || _client.connected() == 0)
            return true;
        if(_client.available())
            receive(_client.readString());
        return _client.connected() == 0;
    }

    virtual void receive(String data) {}
    virtual void send(String data) {
        _client.print(data);
    }

private:
    WiFiClient _client;
};

#endif