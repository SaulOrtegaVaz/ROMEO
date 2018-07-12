#include "control-ap.hh"
#include "general.hh"
#include <EEPROM.h>

// *** VARIABLES GLOBALES ***
WiFiServer servidor(80);
ControlAP apcontrol;

ControlAP::ControlAP(){
    ssid = "Control";
    pass = "123456789";
    IPAddress ip(192,168,5,1);
    IPAddress gate(192,168,5,1);
    IPAddress subnet(255,255,255,0);
    this->ip = ip;
    this->gate = gate;
    this->subnet = subnet;

    ap_estado = false;
    eeprom = 128;
    datosRedex = "";
}

//1) Funciones AP
void ControlAP::iniciarAP(){
    if(!WiFi.softAP(ssid.c_str(), pass.c_str())){
        ap_estado = false;
    }
    if(!WiFi.softAPConfig(ip, gate, subnet)){
        ap_estado = false;
    }
    ap_estado = true;
    EEPROM.begin(eeprom);
}

bool ControlAP::getStaConectados(){
    if(!ap_estado){
        return false;
    }
    if(WiFi.softAPgetStationNum() == 0){
        return false;
    }
    return true;
}

bool ControlAP::getAP(){
    return ap_estado;
}

//2) Funciones Redex
// Almacen:
bool ControlAP::guardarRedex(String datos){
    borrarRedex(eeprom);
    if ((datos.indexOf("ssid=") > -1) && (datos.indexOf("pass=") > -1)){
        for (unsigned i=0; i<datos.length(); i++){
            EEPROM.write(i, datos[i]);
        }
        EEPROM.commit();
        return true;
    }
    return false;
}

void ControlAP::borrarRedex(int n_bytes){
    for (int i=0; i<n_bytes; i++){
        EEPROM.write(i,0);
    }
    EEPROM.commit();
}

String ControlAP::leerRedex(){
    int i=0;
    String s = "";
    while ((char) EEPROM.read(i)){
        s += (char) EEPROM.read(i);
        i++;
    }
    return s;
}

//Datos:
String ControlAP::clienteDatosRedex(WiFiClient cliente){
    String s = "";
    while(cliente.available()){ // Captura el mensaje enviado por el cliente(PC) al servidor(Control)
        s += (char) cliente.read();
    }
    return s.substring(s.indexOf("GET"), s.indexOf("\n")); // Separa del mensaje la parte que contiene los datos pasados por GET
}

void ControlAP::getDatosRedex(String s){
    if ((s.indexOf("ssid=") > -1)&&(s.indexOf("pass=") > -1)){
    this->datosRedex = "";
    this->datosRedex += s.substring(s.indexOf("ssid="), s.indexOf("&", s.indexOf("ssid="))+1);
    this->datosRedex += s.substring(s.indexOf("pass="), s.indexOf("&", s.indexOf("pass="))+1);
    }
}

void ControlAP::webRedex(){
    WiFiClient cliente = servidor.available();
    if (cliente){ // Comprueba si hay cliente web (PC)
        String datosCliente = clienteDatosRedex(cliente); // Obtiene peticion de cliente (PC)
        getDatosRedex(datosCliente); // Separa los datos pasados por PC (SSID y PASS de la red)
        cliente.println(general.paginaWeb(general.leerArchivo("/webRedex.html"))); // El servidor responde a cliente (control) con la pagina web
        cliente.stop(); // El cliente para la comunicacion
    }
    if(datosRedex != ""){
        guardarRedex(datosRedex);
    }
}
