#ifndef DEVICE_HH
#define DEVICE_HH

#include <ESP8266WiFi.h>
#include <functional>

/*
ROMEODevice = modulo del robot (control, comunicacion, ... etc)
Element = dispositivos conectados a los modulos (Sensores, motores, ... etc)

runCmd(X,X,X) --> separa el mensaje pasado en sus diferentes partes como array.
runCmd(X,X) --> filtra mensajes que no son para el ROMEODevice, y realiza las operaciones pedidas

argv = referencia al mensaje pasado en la orden, en las funciones

Ordenes disponibles:
W --> Escribir en el elemento (Element) conectado al dispositivo (ROMEODevice) un valor dado
R --> Leer un valor dado por el elemento conectado al dispositivo
N --> Notificar del resultado de la lectura al dispositivo que la pidio
L --> Listado de los elementos conectados al módulo

Estructura del mensaje --> Array de 5 elementos:
[Orden] [id_DeviceEmisor] [id_DeviceReceptor] [id_ElementReceptor/es] [Datos]

(Ejemplos):
W A B C XXX     Escribe en el elemento C del módulo o dispositivo B el valor XXX. Lo pide el módulo A
R A B C         Lee un valor pasado por el elemento C del módulo o dispositivo B.  Lo pide el módulo A
N B A C XXX     Respuesta a lectura, informa del valor XXX del elemento C del módulo B. Se lo envía a A
L A B           Ofrece la lista de elementos conectados en el modulo B. Lo pide el modulo A

Se pueden señalar para un mensaje varios elementos receptores separandolos con ','
Ejemplo: [sensor1,sensor2,sensor3 ....]
*/

struct Element {
public:
    Element(const char* fmt, unsigned char index);
    virtual ~Element() {}
    const char* elementID(); // Devuelve el id del elemento
    virtual void read(WiFiClient& client, const char** argv) {} // Op. de lectura de datos
    virtual void write(WiFiClient& client, const char** argv) {} // OP. de escritura de datos
    void notify(WiFiClient& client, const char** argv); // Op. de aviso y aviso de lectura
    
protected:
    const char* _id; // Nombre del elemento en la red
};

struct ROMEODevice {
public:
    ROMEODevice() : ROMEODevice("device") { }

    ROMEODevice(const char* id) : _id(id) {
        for (auto& e: _e) e = static_cast<Element*>(nullptr);
    }

    ROMEODevice(const char* id, const char* cfg) : ROMEODevice(id) { 
        configure(cfg); 
    }

    void runCmd(WiFiClient& client, const char** argv); // Ejecuta el protocolo de comunicacion
    void runCmd(WiFiClient& client, char* cmdline, size_t n); // Obtiene el mensaje en forma de array

private:
    void configure(const char* cfg); // Configura un dispositivo (modulo) y sus elementos conectados

private:
    Element* _e[3]; // Lista de elementos conectados en el dispositivo (modulo)
    const char* _id; // Nombre del dispositivo (modulo) en la red
};

#endif