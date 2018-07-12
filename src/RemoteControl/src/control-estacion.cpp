#include "control-estacion.hh"
#include "control-ap.hh"
#include "general.hh"
#include <WString.h>

ControlEstacion estcontrol;

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

static void comprobarRedexCb(ControlEstacion* c) { return c->comprobarRedex(); }

void ControlEstacion::conexionRedex(String ssid, String pass){
  WiFi.begin(ssid.c_str(), pass.c_str());
  contador = 20;
  temporizador.attach_ms(500, comprobarRedexCb, this);
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
