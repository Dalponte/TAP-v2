#include <Arduino.h>
#include <Automaton.h>
#include "setup.h" // Include the setup header

Atm_button flow; // Button state machine
Atm_button rfid; // Button state machine

Atm_led valve;
Atm_led led;
Atm_led led_blue;  // Blue LED state machine
Atm_led led_green; // Green LED state machine
Atm_led led_red;   // Red LED state machine

void setup()
{
  Serial.begin(9600);
  initialize(flow, rfid, valve, led, led_blue, led_green, led_red);

  flow.onPress([](int idx, int v, int up)
               {
    Serial.println("Flowmeter button pressed");
    led_green.trigger(led_green.EVT_TOGGLE); });

  rfid.onPress([](int idx, int v, int up)
               {
    Serial.println("RFID button pressed");
    led_blue.trigger(led_blue.EVT_TOGGLE); });
}

void loop()
{
  automaton.run();
}
