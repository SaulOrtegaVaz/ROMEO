#ifndef FUN_GENERAL_HH
#define FUN_GENERAL_HH

#include <Ticker.h>
#include <WString.h>

class funGeneral {
  private:
    int pinLed;
    int contador;
    Ticker temporizador;
    
  public:
    funGeneral();
    void conmutaLed();
    void parpadeoLed(int);

    // Otros:
    String paginaWeb(String);
    String leerArchivo(String);
    void escribirArchivo(String, String);
};

extern funGeneral general;

#endif