#ifndef SETUP_H
#define SETUP_H

#include <Automaton.h>

// Pin definitions
#define PIN_LED 13
#define PIN_BLUE 44
#define PIN_GREEN 46
#define PIN_RED 45
#define FLOWMETER_PIN 21         // Flowmeter button simulation pin
#define VALVE_PIN 22             // Mosfet valve simulation pin
#define RFID_BUTTON 23           // RFID button simulation pin
#define INITIAL_TIMEOUT_MS 10000 // Initial timeout in milliseconds
#define CONTINUE_TIMEOUT_MS 3000 // Continue timeout in milliseconds

// Function declaration
void initialize(
    Atm_button &rfid,
    Atm_led &valve,
    Atm_led &led,
    Atm_led &led_blue,
    Atm_led &led_green,
    Atm_led &led_red);

#endif // SETUP_H
