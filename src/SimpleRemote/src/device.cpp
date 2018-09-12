#include "device.hh"
#include <ESP8266WiFi.h>

class DistanceSensorElement: public Element {
public:
    DistanceSensorElement(uint8_t out, uint8_t in)
    : _out(out), _in(in) {
        pinMode(_out, OUTPUT);
        pinMode(_in, INPUT);
        digitalWrite(_out, LOW);
    }

    void read(WiFiClient& client, const char** argv) override {
        digitalWrite(_out, HIGH); delayMicroseconds(10); digitalWrite(_out, LOW);
        unsigned t = pulseIn(_in, HIGH) / 59L;
        char val[16];
        snprintf(val, sizeof(val), "%u", t);
        const char* args[] = {argv[1], argv[0], argv[2], val, nullptr};
        notify(client, &args[0]);
    }

private:
    uint8_t _out, _in;
};

class MotorElement: public Element {
public:
    MotorElement(uint8_t out, uint8_t dir)
    : _out(out), _dir(dir) {
        pinMode(_out, OUTPUT);
        pinMode(_dir, OUTPUT);
        digitalWrite(_out, LOW);
        digitalWrite(_dir, LOW);
    }

    void write(WiFiClient& client, const char** argv) override {
        uint8_t v = atoi(argv[3]);
        digitalWrite(_out, v & 1);
        digitalWrite(_dir, v >> 1);
    }

private:
    uint8_t _out, _dir;
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
ROMEODevice::runCmd(WiFiClient& client, char* cmdline, size_t n)
{
    char *p;
    if (n < 1) return;
    const char* argv[4] = { nullptr };
    for (uint8_t i = 0; i < 4; ++i) {
        argv[i] = strtok_r(i? nullptr: cmdline, " \t\r\n", &p);
        if (argv[i] == nullptr) break;
    }
    runCmd(client, argv);
}

void 
Element::notify(WiFiClient& client, const char** argv) {
    char buf[128];
    size_t n = snprintf(buf, sizeof(buf), "N %s %s %s %s", 
                        argv[0], argv[1], argv[2], argv[3]);
    client.write(buf, n);
}


void 
ROMEODevice::configure(const char* cfg)
{
    static uint8_t pin_D[] = { // pins assigned to D0..D8
        16, 5, 4, 0, 2, 14, 12, 13, 15
    };

    for (auto& e: _e) 
        if (e != nullptr) {
            delete e;
            e = static_cast<Element*>(nullptr);
        }

    int n = 0;
    for (const char* p = cfg; *p != '\0'; p+=3) {
        uint8_t a = pin_D[p[1] - '0'];
        uint8_t b = pin_D[p[2] - '0'];
        if (p[0] == 'M') _e[n++] = new MotorElement(a,b);
        else if (p[0] == 'D') _e[n++] = new DistanceSensorElement(a,b);
    }
}
