#ifndef CONTROL_AP_HH
#define CONTROL_AP_HH

#include <ESP8266WiFi.h>

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
};

extern ControlAP apcontrol;
extern WiFiServer servidor;

#endif