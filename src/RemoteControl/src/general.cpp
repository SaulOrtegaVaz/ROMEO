#include "general.hh"
#include <EEPROM.h>
#include <WString.h>
#include <FS.h>


funGeneral general;

funGeneral::funGeneral(){
    pinLed = 2;
}

// LED:
void funGeneral::conmutaLed() {
    digitalWrite(pinLed, !digitalRead(pinLed));
    if (contador--) return;
    digitalWrite(pinLed, HIGH);
    temporizador.detach();
}

static void conmutaLedCb(funGeneral* g) { g->conmutaLed(); }

void funGeneral::parpadeoLed(int repeticiones){
    contador = repeticiones*2;
    temporizador.attach_ms(250, &conmutaLedCb, this);
}

// Lectura de SPIFFS
String funGeneral::leerArchivo(String path){
    String s = "";
    File f;
    SPIFFS.begin();
    f = SPIFFS.open(path, "r");
    if(f){
        while(f.available()){
        s += (char) f.read();   
        }
        f.close();
    }
    SPIFFS.end();
    return s;
}

// Escritura de SPIFFS --> Borra el contenido previo y escribe lo que se pase
void funGeneral::escribirArchivo(String path, String datos){
    File f;
    SPIFFS.begin();
    f = SPIFFS.open(path, "w");
    if(f){
        f.println(datos);
        f.close();
    }
    SPIFFS.end();
}

// Funcion Respuesta del servidor, como pagina web
String funGeneral::paginaWeb(String html){ 
    String request = "";
    String head = "";
    head += "HTTP/1.1 200 OK\r\n";
    head += "Content-Type: text/html\r\n";
    head += "Connection: close\r\n";
    head += "\r\n";
    request += head;
    request += html;
    return request;
}
