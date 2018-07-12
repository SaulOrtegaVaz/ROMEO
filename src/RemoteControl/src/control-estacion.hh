#ifndef CONTROL_ESTACION_HH
#define CONTROL_ESTACION_HH

#include <ESP8266WiFi.h>
#include <Ticker.h>

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
    String obtenerOrden(WiFiClient);
    int codigoOrden(String);
    
  public:
    ControlEstacion();
    void comprobarRedex();
    
    // 1) Funciones de Control de ssid y pass
    void obtenerRedex(String); // Obtiene los datos de la Redex, almacenandolos en las variables ssid y pass
    
    // 2) Funciones de Redex
    void conexionRedex(String, String); // Conexion a la Redex -> con Temporizador
    void webControl();
    String cambiarWeb(int);
    void setSensor();
};

extern ControlEstacion estcontrol;

#endif
