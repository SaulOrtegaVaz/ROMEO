/* 
 *  DATOS IMPORTANTES *
 ******************************************************************************************************************
 *** USO DE STRING mediate entrada de teclado PARA CONECTARSE A LAS REDES ***
 Conexion mediante variables tipo STRING => WiFi.begin(ssid.substring(0, ssid.length()-2).c_str(), pass.substring(0, pass.length()-2).c_str());
 Hay que restar 2 a la longitud de la cadena para restar el caracter de fin de cadena \n que contiene cada STRING
 También hay que usar la funcion c_str() para convertir la cadena de tipo STRING a tipo char ???[X] que es el que usa la funcion WiFi.begin()
 ******************************************************************************************************************
 *** USO DEL LED INTEGRADO DEL ESP
 #define LED_BUILTIN 2
 Por defecto esta puesto el pin 16, pero hay que cambiarlo al 2 en el modulo NodeMCU v3
 ******************************************************************************************************************
 *** 
*/

#include <ESP8266WiFi.h>

#define LED_BUILTIN 2

//String ssid = "Novcoker";
//String pass = "54Xocay7JcMa";

IPAddress ipcon(192,168,1,183);
IPAddress subnetcon(255,255,255,0);
IPAddress gatecon(192,168,1,1);

WiFiServer servidor(80);
WiFiClient cliente;

String paginaWeb(String html){
  String web;
  String header = "";
  header += "HTTP/1.1 200 OK\n";
  header += "Content-Type: text/html\n";
  header += "Connection: close\n";
  header += "Refresh: 5\n";
  header += "\n";
  header += "<!DOCTYPE HTML>\n";
  header += "<head>\n";
  header += "<title>ConexionPC-MCU</title>";
  header += "</head>\n";
  header += "<html>\n";

  web = header;
  web += html;
  web += "</html>";
  return web;
}

// Conexion a red
void conexionRed(){
  WiFi.begin(ssid.c_str(), pass.c_str());
  int tiempo = millis();
  while((millis() - tiempo) < 10000){
    if (WiFi.isConnected()){
      break;
    }
    delay(500);
  }
  if (WiFi.isConnected()){
    while(WiFi.isConnected()){
      conectado(); 
    }
  } else {
    noConectado();
  }
}

// MODO CONECTADO A LA RED WiFi
void conectado(){
  Serial.println("Estado: CONECTADO.");
  aviso_led(WiFi.status());
  Serial.println("Servidor activo.");
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());

  cliente = servidor.available();
  if (cliente){
    String s = "";
    Serial.println("Cliente conectado.");
    while(cliente.connected()){
      if(cliente.available()){
        s += (char) cliente.read();
      } else {
        if (s.indexOf("?") > -1){
          int i_ini = s.indexOf("[")+1;
          int i_fin = s.indexOf("]");
          s = s.substring(i_ini, i_fin);
          Serial.print("Dato del cliente: ");
          Serial.println(s);
          if (s.equals("on") || s.equals("ON")){
            Serial.println("LED ON.");
            digitalWrite(LED_BUILTIN, LOW);
            delay(5000);
            digitalWrite(LED_BUILTIN, HIGH);
          }
        }
        String pag = "<h1>PAGINA DE PRUEBA... --> H1</h1>";
        cliente.println(paginaWeb(pag));
        if (s.equals("wifioff")){
          WiFi.disconnect();
        }
        break;
      }
    }
    delay(100);
    cliente.stop();
    Serial.println("Cliente desconectado.");
  } else {
    Serial.println("No hay clientes");
    delay(3000);
  }
}

// MODO NO CONECTADO A LA RED WiFi
void noConectado(){
  Serial.println("Estado: NO CONECTADO.");
  aviso_led(WiFi.status());
  delay(250);
  WiFi.disconnect();
  delay(250);
}

// Aviso del estado de la conexion a la red WiFi mediante parpadeos de LED segun el estado
void aviso_led(int est_conexion){
  for (int i=0; i<est_conexion; i++){
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  servidor.begin();
}

void loop() {
  conexionRed();
}
