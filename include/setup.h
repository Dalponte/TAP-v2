#ifndef SETUP_H
#define SETUP_H

#include <Automaton.h>

// Pin definitions
#define PIN_LED 13
#define PIN_BLUE 44
#define PIN_GREEN 46
#define PIN_RED 45
#define FLOWMETER_PIN 21 // Flowmeter button simulation pin
#define VALVE_PIN 22     // Mosfet valve simulation pin
#define BUTTON 23        // Button to trigger the pour

// Pour process timing
#define INITIAL_TIMEOUT_MS 10000
#define CONTINUE_TIMEOUT_MS 3000
#define FLOW_UPDATE_INTERVAL_MS 500

void initialize(
    Atm_button &button,
    Atm_led &valve,
    Atm_led &led,
    Atm_led &led_blue,
    Atm_led &led_green,
    Atm_led &led_red);

#endif // SETUP_H
