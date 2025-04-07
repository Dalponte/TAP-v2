#include <Arduino.h>
#include <Automaton.h>
#include "setup.h"       // Include the setup header
#include "PourMachine.h" // Include our simplified PourMachine

Atm_led valve;
Atm_led led;
Atm_led led_blue;  // Blue LED state machine
Atm_led led_green; // Green LED state machine
Atm_led led_red;   // Red LED state machine

Atm_digital flowmeter; // Button state machine
PourMachine pour;      // Our simplified PourMachine

Atm_button rfid; // Button state machine

void flow(int idx, int v, int up)
{
  led.trigger(led.EVT_TOGGLE); // Toggle the LED on flow event
  pour.flow();
}

void onCounterReached(int idx, int v, int up)
{
  Serial.print("Counter reached: ");
  Serial.println(v);
}

void setup()
{
  Serial.begin(9600);
  initialize(rfid, valve, led, led_blue, led_green, led_red);

  // Use the constants from setup.h for the timeouts
  pour.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS);
  pour.trace(Serial); // Enable tracing to see state transitions

  flowmeter.begin(FLOWMETER_PIN, 1, false, true)
      .onChange(HIGH, flow);

  rfid.onPress([](int idx, int v, int up)
               {
                 int pour_pulses = 50;
                 pour.start(pour_pulses); });
}

void loop()
{
  automaton.run();
}
