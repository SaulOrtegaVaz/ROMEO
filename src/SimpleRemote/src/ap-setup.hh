#ifndef AP_SETUP_HH
#define AP_SETUP_HH

#include <ESP8266WiFi.h>

class APSetup {
public:
    APSetup() : _state(State::Disconnected) {}

    bool run() {
        switch(_state) {
        case State::Disconnected:
            if (WiFi.softAP("Control", "12345678"))
                _state = State::Associated;
            break;
        case State::Associated:
            break;
        }
        return _state == State::Associated;
    }

private:
    enum class State {
        Disconnected,
        Associated,
    };

    State _state;
};

#endif