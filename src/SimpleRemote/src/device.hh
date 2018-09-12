#ifndef DEVICE_HH
#define DEVICE_HH

#include <ESP8266WiFi.h>

/* Define objeto ROMEODevice que 
- se configura
  - Motor/es
  - Sensores
- define runCmd(argv)
  - filtra mensajes que no son para él
  - delega en componentes el trabajo real 
*/

/*
W 0 1 3 0.5  Escribe en el dispositivo 3 del módulo 1 un 0.5. Lo pide el módulo 0
R 1 2 0 Lee el dispositivo 0 del módulo 2.  Lo pide el módulo 1
N 2 1 0 0.534  Responde a lectura informando del valor del dispositivo 0 del módulo 2. Se lo envía al 1
*/

struct Element {
    virtual ~Element() {}
    virtual void read(WiFiClient& client, const char** argv) {}
    virtual void write(WiFiClient& client, const char** argv) {}
protected:
    void notify(WiFiClient& client, const char** argv);
};

struct ROMEODevice {
    ROMEODevice() : ROMEODevice("device") { }

    ROMEODevice(const char* id) : _id(id) {
        for (auto& e: _e) e = static_cast<Element*>(nullptr);
    }

    ROMEODevice(const char* id, const char* cfg) : ROMEODevice(id) { 
        configure(cfg); 
    }

    void runCmd(WiFiClient& client, const char* argv[]);
    void runCmd(WiFiClient& client, char* cmdline, size_t n);

private:
    void configure(const char* cfg);

private:
    Element* _e[8];
    const char* _id;
};

#endif