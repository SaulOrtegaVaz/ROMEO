#include "device.hh"
#include <ESP8266WiFi.h>

// --------------------- ELEMENT ----------------------------------

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

// --------------------- TIPOS de ELEMENT ----------------------------------

class DistanceSensorElement: public Element { // Sensor de distancia ultrasonidos
public:
    DistanceSensorElement(int index, uint8_t out, uint8_t in)
    : Element("distance%d", index), _out(out), _in(in) {
        pinMode(_out, OUTPUT);
        pinMode(_in, INPUT);
        digitalWrite(_out, LOW);
    }

    void read(WiFiClient& client, const char** argv) override {
        unsigned lim = 30; // cm de distancia hacia el sensor para activar señal
        char data[32];
        uint8_t state = 0; // estado del sensor HIGH 1 o LOW 0

        // Obtencion de la señal del sensor
        digitalWrite(_out, HIGH); delayMicroseconds(10); digitalWrite(_out, LOW);
        unsigned d = pulseIn(_in, HIGH) / 59L;

        // Ejecucion de operaciones con la señal obtenida
        if (d < lim){
            state = 1;
            snprintf(data, sizeof(data), "W %s control motor0,motor1 0\n", argv[1]);
            client.write(data, sizeof(data));
        }

        snprintf(data, sizeof(data), "Sensor: %u\n", state);
        argv[3] = data;
        notify(client, argv);
        Serial.print(data);
    }

private:
    uint8_t _out, _in;
};

class MotorElement: public Element { // Motores DC
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
        char data[32];

        // Activacion de los motores
        digitalWrite(_out, v & 1);

        snprintf(data, sizeof(data), "Motor: %d\n", digitalRead(_out));
        argv[3] = (const char*) data;
        notify(client, argv);
        Serial.print("Estado del motor: ");
        Serial.println(digitalRead(_out) ? "Activado.":"Desactivado.");

        // Sentido de giro de los motores
        digitalWrite(_dir, v >> 1);

        snprintf(data, sizeof(data), "Giro: %d\n", digitalRead(_dir));
        argv[3] = (const char*) data;
        notify(client, argv);
        Serial.print("Direccion del motor: ");
        Serial.println(digitalRead(_dir) ? "Activado.":"Desactivado.");

        snprintf(data, sizeof(data), "R %s com distance0,distance1 0\n", argv[1]);
        client.write(data, sizeof(data));
    }

private:
    uint8_t _out, _dir;
};

// --------------------- DEVICE ----------------------------------

// Mete lista de elementos destino, pasados en la orden, en un const char* dest[3]
static void get_destination_elements(const char* dest[], char* elstring);
// Comprueba si el elemento dado en la orden esta en los elementos conectados en el modulo
static bool is_destination_element(const char* id, const char** dest);

void 
ROMEODevice::runCmd(WiFiClient& client, const char** argv)// Protocolo de comunicacion
{
    if(argv[2] == nullptr) {
        char er[] = "ERROR: Mensaje invalido.\n";
        client.write(er, sizeof(er));
        return;
    }

    // Filtro para mensajes que no son mios
    if (strcmp(argv[2], _id)) return;

    // Encuentra los elementos de destino en el mensaje
    const char* dest[3] = { nullptr }; // lista de elementos destino
    get_destination_elements(dest, (char*)argv[3]);

    // Ejecuta la operacion solicitada para cada elemento destino
    auto dispatchCommand = [&](const char* cmd, std::function<void(Element*, const char**argv)> func) {
        if (strcmp(cmd, argv[0])) return;
        for(auto& e: _e) {
            if (e == nullptr) break;
            if (is_destination_element(e->elementID(), dest)) {
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
        argv[3] = "none";
        argv[4] = e->elementID();
        Serial.print("List elements:");
        Serial.println(argv[4]);
        e->notify(client, argv+1);
    });
}

static void
get_destination_elements(const char* dest[], char* elstring)
{
    if (elstring == nullptr || elstring[0] == '\0') return;
    char *p;

    for(uint8_t i = 0; i < 3; ++i){
        dest[i] = strtok_r(i? nullptr: elstring, ",", &p);
        if (dest[i] == nullptr) break;
    }
}

static bool
is_destination_element(const char* id, const char** dest)
{
    if (*dest == nullptr) return true;
    for (;*dest != nullptr; ++dest) {
        if (0 == strcmp(id, *dest)) return true;
    }
    return false;
}

void 
ROMEODevice::runCmd(WiFiClient& client, char* cmdline, size_t n)
{
    char *p;
    const char* argv[5] = { nullptr };

    if (n < 2) {
        char er[] = "ERROR: Mensaje vacio.\n";
        client.write(er, sizeof(er));
        return;
    }
    cmdline[n] = '\0';
    
    for (uint8_t i = 0; i < 5; ++i) {
        argv[i] = strtok_r(i? nullptr: cmdline, " \t\r\n", &p);
        if (argv[i] == nullptr) break;
    }

    runCmd(client, argv);
}

void 
ROMEODevice::configure(const char* cfg)
{
    static uint8_t pin_D[] = { // Equivalencia de pines NodeMCU-ESP8266
        16, 5, 4, 0, 2, 14, 12, 13, 15
    };

    for (auto& e: _e) { // Limpia lista de elementos conectados
        if (e != nullptr) {
            delete e;
            e = static_cast<Element*>(nullptr);
        }
    }

    int n = 0;

    for (const char* p = cfg; *p != '\0'; p+=3) { // Configura elementos conectados

        uint8_t a = pin_D[p[1] - '0'];
        uint8_t b = pin_D[p[2] - '0'];

        if (n >= 3) return; // Limite de elementos posibles de conectar en el device

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