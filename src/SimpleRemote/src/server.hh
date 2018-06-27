#ifndef SERVER_HH
#define SERVER_HH

#include "ap-setup.hh"
#include "client.hh"

class ROMEOServer {
public:
    ROMEOServer() : _server(80), _state(State::MissingAP) {}

    bool run() {
        switch(_state) {
        case State::MissingAP:
            if (_ap.run())
                _state = State::Associated;
            break;
        case State::Associated:
            _server.begin();
            _state = State::Listening;
            break;
        case State::Listening:
            break;
        } 
        return _state == State::Listening;      
    }

    ROMEOClient available() {
        return ROMEOClient(_server.available());
    }

private:
    enum class State {
        MissingAP,
        Associated,
        Listening,
    };

    APSetup _ap;
    WiFiServer _server;
    State _state;
};

#endif