#include "device.hh"
#include <ESP8266WiFi.h>

class DistanceSensorElement: public Element {
public:
    virtual void setup(uint8_t pin) override {
        pinMode(pin, OUTPUT);
        pinMode(pin + 1, INPUT);
        digitalWrite(pin, LOW);
    }
    void read(WiFiClient& client, const char** argv) override {
        uint8_t pin = atoi(argv[2]);
        digitalWrite(pin, HIGH); delayMicroseconds(10); digitalWrite(pin, LOW);
        unsigned t = pulseIn(pin + 1, HIGH) / 59L;
        char val[16];
        snprintf(val, sizeof(val), "%u", t);
        const char* args[] = {argv[1], argv[0], argv[2], val, nullptr};
        notify(client, &args[0]);
    }

    void write(WiFiClient& client, const char** argv) override {}
};

class MotorElement: public Element {
public:
    virtual void setup(uint8_t pin) override {
        pinMode(pin, OUTPUT);
        pinMode(pin + 1, OUTPUT);
        digitalWrite(pin, LOW);
        digitalWrite(pin + 1, LOW);
    }

    void read(WiFiClient& client, const char** argv) override {
    }

    void write(WiFiClient& client, const char** argv) override {
        uint8_t pin = atoi(argv[2]);
        uint8_t v = atoi(argv[3]);
        digitalWrite(pin, v & 1);
        digitalWrite(pin + 1, v >> 1);
    }
};

  
void 
ROMEODevice::runCmd(WiFiClient& client, const char** argv)
{
    // filtra mensajes que no son míos
    if (0 != strcmp(argv[2], _id)) return;
    // Encontrar elemento
    Element* e = _e[atoi(argv[3])];
    if (e == nullptr) return;
    if (0 == strcmp("R", argv[0]))
        e->read(client, argv+1);
    else if (0 == strcmp("W", argv[0]))
        e->write(client, argv+1);
}

void 
Element::notify(WiFiClient& client, const char** argv) {
    char buf[128];
    size_t n = snprintf(buf, sizeof(buf), "V %s %s %s %s", 
                        argv[0], argv[1], argv[2], argv[3]);
    client.write(buf, n);
}


void 
ROMEODevice::configure(const char* id, const char* cfg)
{
    static DistanceSensorElement DistanceSensor;
    static MotorElement Motor;
    static MotorElement Element;

    _id = id;
    for (uint8_t i = 0; i < sizeof(_e)/sizeof(_e[0]); ++i) {
        if (cfg[i] == '\0') return;
        else if (cfg[i] == 'M') _e[i] = &Motor;
        else if (cfg[i] == 'D') _e[i] = &DistanceSensor;
        else _e[i] = &Element;
        _e[i]->setup(i);
    }
}
