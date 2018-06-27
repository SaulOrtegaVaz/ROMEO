/*

Periodicamente al servidor hay que pedir available, que devuelve un WiFiClient

ese WiFiClient hay que añadirlo al reactor

Cuando un WiFiClient está desconectado hay que quitarlo del reactor haciendo un .stop() antes

En principio puede haber dos clientes: módulo de comunicación y PC

*/