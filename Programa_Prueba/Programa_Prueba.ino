/* 
 //WiFi.begin(ssid.c_str(), pass.c_str());
 Redex = Red WiFi externa Control<->PC
 Redin = Red WiFi interna Control<->Comunicacion
*/
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <EEPROM.h>
#include "FS.h"

// *** VARIABLES GLOBALES ***
WiFiServer servidor(80);

// Funcion para obtener el archivo html de una pagina web almacenado en SPIFF
String getHTML(String archivo){
  SPIFFS.begin();
  if(SPIFFS.exists(archivo)){
    String web = "";
    File f = SPIFFS.open(archivo, "r");
    while(f.available()){
      web += (char) f.read();
    }
    f.close();
    SPIFFS.end();
    return web;
  }
  SPIFFS.end();
  return "";
}

// Funcion Respuesta del servidor, como pagina web
String paginaWeb(String html){ 
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

//***************************************************************************************
//CLASE PUNTO DE ACCESO CONTROL --> RED INTERNA
class ControlAP{
  private:
    // Datos red interna
    String ssid; // ssid de AP
    String pass; // pass de AP
    IPAddress ip;
    IPAddress gate;
    IPAddress subnet;
    
    // Datos red externa
    String datosRedex; // Datos obtenidos por web de la red externa
    
    // Otros
    int eeprom; // Bytes reservados para memoria eeprom
    bool ap_estado; // Estado del punto de acceso True = on, False = off

    //2) Funciones Almacen de Redex
    bool guardarRedex(String); // Guardar en EEPROM los datos de la red externa
    void borrarRedex(int); // Borrar EEPROM, para cambiar los datos de la red externa
    
    //3) Funciones Obtencion de datos de Redex
    String clienteDatosRedex(WiFiClient); // Funcion que obtiene los datos de la Redex de un cliente web(PC)
    void getDatosRedex(String); // Funcion que separa los datos de la Redex pasados por el cliente del resto de datos
    
  public:
    ControlAP();

    //1) Funciones Control AP
    void iniciarAP(); // Funcion que inicia el AP
    bool getStaConectados(); // Funcion que comprueba si hay o no sta conectados True = sta On, False = sta OFF
    bool getEstado(); // Funcion que muestra el estado del AP

    //2) Funciones Almacen de Redex
    int mem_eeprom(); // Funcion que devuelve la cantidad de memoria eeprom activada (en este caso 128 bytes)
    String leerRedex(); // Leer EEPROM para obtener los datos de la red externa

    //3) Funciones Obtencion de datos de Redex
    void Redex(); // Funcion para guardar los datos de la red externa, agrupados en una sola String en forma: ssid=XXX&pass=XXX&
};

ControlAP::ControlAP(){
  IPAddress ip(192,168,5,1);
  IPAddress gate(192,168,5,1);
  IPAddress subnet(255,255,255,0);
  this->ip = ip;
  this->gate = gate;
  this->subnet = subnet;
  
  this->ssid = "Control";
  this->pass = "123456789";
  this->datosRedex = "";
  this->eeprom = 128;

  EEPROM.begin(this->eeprom);
}

//1) Funciones Control AP
void ControlAP::iniciarAP(){
  if(!WiFi.softAP(this->ssid.c_str(), this->pass.c_str())){
    this->ap_estado = false;
  }
  if(!WiFi.softAPConfig(this->ip, this->gate, this->subnet)){
    this->ap_estado = false;
  }
  this->ap_estado = true;
}

bool ControlAP::getStaConectados(){
  if(!this->ap_estado){
    return false;
  }
  if(WiFi.softAPgetStationNum() == 0){
    return false;
  }
  return true;
}

bool ControlAP::getEstado(){
  return this->ap_estado;
}

//2) Funciones Almacen de Redex
int ControlAP::mem_eeprom(){
  return this->eeprom;
}

bool ControlAP::guardarRedex(String datos){
  this->borrarRedex(this->eeprom);
  if ((datos.indexOf("ssid=") > -1) && (datos.indexOf("pass=") > -1)){
    for (int i=0; i<datos.length(); i++){
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
  while (((char) EEPROM.read(i)) != '0'){
    s += (char) EEPROM.read(i);
    i++;
  }
  return s;
}

//3) Funciones Obtencion de datos de Redex
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

void ControlAP::Redex(){
  WiFiClient cliente = servidor.available();
  if (cliente){ // Comprueba si hay cliente web (PC)
    String datosCliente = this->clienteDatosRedex(cliente); // Obtiene peticion de cliente (PC)
    this->getDatosRedex(datosCliente); // Separa los datos pasados por PC (SSID y PASS de la red)
    cliente.println(paginaWeb(getHTML("/web.html"))); // El servidor responde a cliente (control) con la pagina web
    cliente.stop(); // El cliente para la comunicacion
  }
  if(this->datosRedex != ""){
    this->guardarRedex(this->datosRedex);
  }
}

//***************************************************************************************
//CLASE ESTACION CONTROL --> RED EXTERNA
class ControlEstacion{
  private:
    String ssid; // ssid externo
    String pass; // pass externo

    // 2) Funciones de Control de ssid y pass
    String verSSID(); // Funcion que devuelve el ssid registrado de la red externa
    String verPASS(); // Funcion que devuelve el pass registrado de la red externa
    String getSSID(String); // Funcion que obtiene el dato del SSID de la red a conectar el modulo de control, dentro de los datos de la red almacenados
    String getPASS(String); // Funcion que obtiene el dato del PASS de la red a conectar el modulo de control, dentro de los datos de la red almacenados
  public:
    ControlEstacion();
    
    // 1) Funciones de Redex
    // 2) Funciones de Control de ssid y pass
    void obtenerRedex(String); // Obtiene los datos de la Redex, almacenandolos en las variables ssid y pass
};

ControlEstacion::ControlEstacion(){
  this->ssid = "";
  this->pass = "";
}

// 2) Funciones de Control de ssid y pass
String ControlEstacion::verSSID(){
  return this->ssid;
}

String ControlEstacion::verPASS(){
  return this->pass;
}

String ControlEstacion::getSSID(String s){
  String ssid = "";
  int i_ini = s.indexOf("=", s.indexOf("ssid=")) + 1; // Principio del dato
  int i_fin = s.indexOf("&", s.indexOf("ssid=")); // Final del dato
  ssid = s.substring(i_ini, i_fin); // SSID
  return ssid;
}

String ControlEstacion::getPASS(String s){
  String pass = "";
  int i_ini = s.indexOf("=", s.indexOf("pass=")) + 1; // Principio del dato
  int i_fin = s.indexOf("&", s.indexOf("pass=")); // Final del dato
  pass = s.substring(i_ini, i_fin); // PASS
  return pass;
}

void ControlEstacion::obtenerRedex(String s){
  this->ssid = this->getSSID(s);
  this->pass = this->getPASS(s);
}

//***************************************************************************************
// *** PROGRAMA ****
void setup() {
  Serial.begin(115200);
}

void loop() {
}
