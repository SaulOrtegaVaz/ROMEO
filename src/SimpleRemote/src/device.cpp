#include "device.hh"
#include <ESP8266WiFi.h>

Element::Element(const char* fmt, unsigned char index)
{
    char buf[16];
    snprintf(buf, sizeof(buf), fmt, index);
    _id = strdup(buf);
}

const char* 
Element::elementID(){ return _id; }

void 
Element::notify(WiFiClient& client, const char** argv) {
    char buf[128];
    size_t n = snprintf(buf, sizeof(buf), "N %s %s %s %s\r\n", 
                        argv[1], argv[0], argv[2], argv[3]);
    client.write(buf, n);
}

class DistanceSensorElement: public Element {
public:
    DistanceSensorElement(int index, uint8_t out, uint8_t in)
    : Element("distance%d", index), _out(out), _in(in) {
        pinMode(_out, OUTPUT);
        pinMode(_in, INPUT);
        digitalWrite(_out, LOW);
    }

    void read(WiFiClient& client, const char** argv) override {
        digitalWrite(_out, HIGH); delayMicroseconds(10); digitalWrite(_out, LOW);
        unsigned t = pulseIn(_in, HIGH) / 59L;
        char val[16];
        snprintf(val, sizeof(val), "%u", t);
        const char* args[] = {argv[0], argv[1], argv[2], val};
        notify(client, args);
    }

private:
    uint8_t _out, _in;
};

class MotorElement: public Element {
public:
    MotorElement(int index, uint8_t out, uint8_t dir)
    : Element("motor%d", index), _out(out), _dir(dir) {
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

static void get_destination_elements(const char* dest[], char* elstring);
static bool is_destination_element(const char* id, const char** dest);

void 
ROMEODevice::runCmd(WiFiClient& client, const char** argv)
{
    // Filtro para mensajes que no son mios
    if (0 != strcmp(argv[2], _id)) return;

    // Encuentra los elementos de destino en el mensaje
    const char* dest[4] = { nullptr }; // lista de elementos destino
    get_destination_elements(dest, (char*)argv[3]);

    auto dispatchCommand = [&](const char* cmd, std::function<void(Element*, const char**argv)> func) {
        if (strcmp(cmd, argv[0])) return;
        for(auto& e: _e) {
            if (e == nullptr) break;
            const char* id = e->elementID();
            if (is_destination_element(id, dest)) {
                func(e, argv);
            }
        }
    };

    dispatchCommand("R", [&](Element* e, const char**argv) {
        argv[3] = e->elementID();
        Serial.println("Read from:");
        Serial.println(argv[3]);
        e->read(client, argv+1);
    });

    dispatchCommand("W", [&](Element* e, const char**argv) {
        argv[3] = e->elementID();
        Serial.println("Write to:");
        Serial.println(argv[3]);
        e->write(client, argv+1);
    });

    dispatchCommand("L", [&](Element* e, const char**argv) {
        Serial.println("List elements:");
        Serial.println(e->elementID());
    });
}

static void
get_destination_elements(const char* dest[], char* elstring)
{
    if (elstring == nullptr || elstring[0] == '\0') return;
    char *p;
    uint8_t i = 0;
    do {
        dest[i] = strtok_r(i? nullptr: elstring, ",", &p);
        i++;
    } while(dest[i] != nullptr);
}

static bool
is_destination_element(const char* id, const char** dest)
{
    if (*dest == nullptr) 
        return true;
    for (;*dest != nullptr; ++dest) {
        if (0 == strcmp(id, *dest)) 
            return true;
    }
    return false;
}

void 
ROMEODevice::runCmd(WiFiClient& client, char* cmdline, size_t n)
{
    char *p;
    if (n < 1) return;
    cmdline[n] = '\0';
    const char* argv[5] = { nullptr };
    for (uint8_t i = 0; i < 5; ++i) {
        argv[i] = strtok_r(i? nullptr: cmdline, " \t\r\n", &p);
        if (argv[i] == nullptr) break;
    }

#if 0
    auto x = [](const char*p) { return p == nullptr? "null": p; };
    char buf[80];
    sprintf(buf, "[%s][%s][%s][%s][%s]", 
            x(argv[0]),
            x(argv[1]),
            x(argv[2]),
            x(argv[3]),
            x(argv[4])
        );
    Serial.println(buf);
#endif

    runCmd(client, argv);
}

void 
ROMEODevice::configure(const char* cfg)
{
    static uint8_t pin_D[] = { // Equivalencia de pines NodeMCU-ESP8266
        16, 5, 4, 0, 2, 14, 12, 13, 15
    };

    for (auto& e: _e) {
        if (e != nullptr) {
            delete e;
            e = static_cast<Element*>(nullptr);
        }
    }

    int n = 0;

    for (const char* p = cfg; *p != '\0'; p+=3) {

        uint8_t a = pin_D[p[1] - '0'];
        uint8_t b = pin_D[p[2] - '0'];

        if (p[0] == 'M') {
            _e[n] = new MotorElement(n,a,b);
            n++;
        }
        
        if (p[0] == 'D') {
            _e[n] = new DistanceSensorElement(n,a,b);
            n++;
        }
    }
}