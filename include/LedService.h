#ifndef LED_SERVICE_H
#define LED_SERVICE_H

#include <Arduino.h>
#include <Automaton.h>

#define PIN_LED 13 // Built-in LED pin
#define PIN_BLUE 44
#define PIN_GREEN 46
#define PIN_RED 45

class LedService
{
private:
    static LedService *instance;
    LedService(); // Private constructor for singleton

    Atm_led led;
    Atm_led led_blue;
    Atm_led led_green;
    Atm_led led_red;

public:
    static LedService &getInstance();
    void setGreen(bool state);
    void setBlue(bool state);
    void setRed(bool state);

    void green();
    void blue();
    void red();
};

#endif // LED_SERVICE_H