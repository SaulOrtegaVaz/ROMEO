#ifndef LED_HH
#define LED_HH

#include <Arduino.h>

class LEDControl {
public:
    LEDControl(): _count(0) { pinMode(2, OUTPUT); }

    static void on() { digitalWrite(2, LOW); }
    static void off() { digitalWrite(2, HIGH); }
    static void toggle() { digitalWrite(2, !digitalRead(2)); }
    void blink(short n) { if (++_count % n == 0 ) toggle(); }

private:
    short _count;
};

#endif
