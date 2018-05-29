/* 
 //WiFi.begin(ssid.c_str(), pass.c_str());

 ** Tickers correctos **
 temporizador.attach_ms(200, reinterpret_cast<void(*)(Estados*)>(&Estados::conmutaLed), this);
 temporizador.attach(200, reinterpret_cast<void(*)(Estados*)>(&Estados::conmutaLed), this);
 
 Redex = Red WiFi externa Control<->PC
 Redin = Red WiFi interna Control<->Comunicacion
*/
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <EEPROM.h>
#include "FS.h"

// *** VARIABLES GLOBALES ***
WiFiServer servidor(80);

//***************************************************************************************
// FUNCIONES GENERALES
class funGeneral{
  private:
    //LED:
    int pinLed;
    int contador;
    Ticker temporizador;
    
    void conmutaLed();
  public:
    funGeneral();
    
    // LED:
    void parpadeoLed(int);

    // Otros:
    String paginaWeb(String);
    String leerArchivo(String);
    void escribirArchivo(String, String);
} general;

funGeneral::funGeneral(){
  pinLed = 2;
}

// LED:
void funGeneral::conmutaLed(){
  digitalWrite(pinLed, !digitalRead(pinLed));
  if (contador--) return;
  digitalWrite(pinLed, HIGH);
  temporizador.detach();
}

void funGeneral::parpadeoLed(int repeticiones){
  contador = repeticiones*2;
  temporizador.attach_ms(250, reinterpret_cast<void(*)(funGeneral*)>(&funGeneral::conmutaLed), this);
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

//***************************************************************************************
//CLASE PUNTO DE ACCESO CONTROL --> RED INTERNA
class ControlAP{
  private:
    // Datos Redin
    String ssid; // ssid de AP
    String pass; // pass de AP
    IPAddress ip;
    IPAddress gate;
    IPAddress subnet;
    bool ap_estado; // Estado del punto de acceso True = on, False = off
    
    // Otros
    int eeprom; // Bytes reservados para memoria eeprom
    String datosRedex; // Datos obtenidos por web de la red externa
    
    //2) Funciones Redex
    // Almacen:
    bool guardarRedex(String); // Guardar en EEPROM los datos de la red externa
    void borrarRedex(int); // Borrar EEPROM, para cambiar los datos de la red externa
    // Datos:
    String clienteDatosRedex(WiFiClient); // Funcion que obtiene los datos de la Redex de un cliente web(PC)
    void getDatosRedex(String); // Funcion que separa los datos de la Redex pasados por el cliente del resto de datos
    
  public:
    ControlAP();

    //1) Funciones AP
    void iniciarAP(); // Funcion que inicia el AP
    bool getStaConectados(); // Funcion que comprueba si hay o no sta conectados True = sta On, False = sta OFF
    bool getAP(); // Funcion que muestra el estado del AP

    //2) Funciones Redex
    // Almacen:
    String leerRedex(); // Leer EEPROM para obtener los datos de la red externa
    
    // Datos:
    void webRedex(); // Funcion para guardar los datos de la red externa, agrupados en una sola String en forma: ssid=XXX&pass=XXX&
} apcontrol;

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
  this->borrarRedex(eeprom);
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

//***************************************************************************************
//CLASE ESTACION CONTROL --> RED EXTERNA
class ControlEstacion{
  private:
    // Datos de conexion
    String ssid; // ssid externo
    String pass; // pass externo
    int contador;
    Ticker temporizador;
    
    // Otros
    int nord;
    bool est_estado;
    int sensores;
    String webOrigen;
    String webFinal;
    
    // 1) Funciones de Control de ssid y pass
    String getSSID(String); // Funcion que obtiene el dato del SSID de la red a conectar el modulo de control, dentro de los datos de la red almacenados
    String getPASS(String); // Funcion que obtiene el dato del PASS de la red a conectar el modulo de control, dentro de los datos de la red almacenados

    // 2) Funciones de Redex
    void comprobarRedex();
    String obtenerOrden(WiFiClient);
    int codigoOrden(String);
    
  public:
    ControlEstacion();
    
    // 1) Funciones de Control de ssid y pass
    void obtenerRedex(String); // Obtiene los datos de la Redex, almacenandolos en las variables ssid y pass
    
    // 2) Funciones de Redex
    void conexionRedex(String, String); // Conexion a la Redex -> con Temporizador
    void webControl();
    String cambiarWeb(int);
    void setSensor();
} estcontrol;

ControlEstacion::ControlEstacion(){
  ssid = "";
  pass = "";
  nord = 0;
  est_estado = false;
  webOrigen = general.leerArchivo("/webControl.html");
  webFinal = general.leerArchivo("/webControl.html");
  sensores = 0;
}

void ControlEstacion::setSensor(){
  switch(sensores){
    case 1:
      sensores = 10;
      break;
    case 10:
      sensores = 11;
      break;
    case 11:
      sensores = 0;
      break;
    default:
      sensores = 1;
      break;
  }
}

// 1) Funciones de Control de ssid y pass
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
  ssid = getSSID(s);
  pass = getPASS(s);
}

// 2) Funciones de Redex
void ControlEstacion::comprobarRedex(){
  if(WiFi.isConnected()){
    temporizador.detach();
    est_estado = true;
  }
  if(!contador--){
    temporizador.detach();
    est_estado = false;
  }
}

String ControlEstacion::obtenerOrden(WiFiClient cliente){//+++***+++
  String s = "";
  while(cliente.available()){
    s += (char) cliente.read();
  }
  if(s.indexOf("orden") > -1) return s.substring(s.indexOf("=", s.indexOf("orden"))+1, s.indexOf("0", s.indexOf("orden")));
  return "";
}

int ControlEstacion::codigoOrden(String orden){
  if(orden == "auto") return 5;
  if(orden == "avance") return 1;
  if(orden == "retroceso") return 2;
  if(orden == "izquierda") return 3;
  if(orden == "derecha") return 4;
  return 0; // parar
}

String ControlEstacion::cambiarWeb(int sensores){
  String cambios = "";
  String b = "";
  switch (sensores){
    case 1:
      cambios = "cambiosColor(\"red\",\"black\", \"green\",\"white\")";
      webFinal.replace(webFinal.substring(webFinal.indexOf("cambiosColor(\""), webFinal.indexOf(";", webFinal.indexOf("cambiosColor(\""))), cambios);
      break;
    case 10:
      cambios = "cambiosColor(\"green\",\"white\", \"red\",\"black\")";
      webFinal.replace(webFinal.substring(webFinal.indexOf("cambiosColor(\""), webFinal.indexOf(";", webFinal.indexOf("cambiosColor(\""))), cambios);
      break;
    case 11:
      cambios = "cambiosColor(\"green\",\"white\", \"green\",\"white\")";
      webFinal.replace(webFinal.substring(webFinal.indexOf("cambiosColor(\""), webFinal.indexOf(";", webFinal.indexOf("cambiosColor(\""))), cambios);
      break;
    default:
      cambios = "cambiosColor(\"red\",\"black\", \"red\",\"black\")";
      webFinal.replace(webFinal.substring(webFinal.indexOf("cambiosColor(\""), webFinal.indexOf(";", webFinal.indexOf("cambiosColor(\""))), cambios);
      break;
  }
  return webFinal;
}

void ControlEstacion::conexionRedex(String ssid, String pass){
  WiFi.begin(ssid.c_str(), pass.c_str());
  contador = 20;
  temporizador.attach_ms(500, reinterpret_cast<void(*)(ControlEstacion*)>(&ControlEstacion::comprobarRedex), this);
}

void ControlEstacion::webControl(){
  WiFiClient cliente = servidor.available();
  if(cliente){
    String orden = obtenerOrden(cliente);
    if(orden != "") nord = codigoOrden(orden);
    cliente.println(general.paginaWeb(general.leerArchivo("/webControl.html")));
    general.escribirArchivo("/webControl.html", cambiarWeb(sensores));
    cliente.stop();
  }
}
//***************************************************************************************
// *** PROGRAMA ****
// general, apcontrol, estcontrol

void setup() {
  Serial.begin(115200);
  servidor.begin();
  estcontrol.conexionRedex("","");
}
int sensores = 0;
void loop() {
  if(WiFi.isConnected()){
    Serial.println(WiFi.localIP());
    estcontrol.setSensor();
    estcontrol.webControl();
  }
  delay(2000);
}
