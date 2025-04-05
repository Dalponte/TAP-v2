#ifndef TAPMACHINE_H
#define TAPMACHINE_H

#include <Automaton.h>

class TapMachine : public Machine
{
public:
    enum
    {
        INIT,
        IDLE,
        POURING,
        DONE
    }; // STATES
    enum
    {
        EVT_NEXT,
        ELSE
    }; // EVENTS
    enum
    {
        ENT_INIT,
        ENT_IDLE,
        ENT_POURING,
        ENT_DONE
    }; // ACTIONS

    static const int PIN_LED = 13;       // Built-in LED
    static const int PIN_RGB_RED = 44;   // RGB LED - Red
    static const int PIN_RGB_GREEN = 45; // RGB LED - Green
    static const int PIN_RGB_BLUE = 46;  // RGB LED - Blue

    TapMachine() : Machine() {}

    TapMachine &begin();
    int event(int id);
    void action(int id);

private:
    atm_timer_millis timer;
    Atm_led led;       // Built-in LED (PIN_LED)
    Atm_led led_red;   // Red LED
    Atm_led led_green; // Green LED
    Atm_led led_blue;  // Blue LED
};

#endif
