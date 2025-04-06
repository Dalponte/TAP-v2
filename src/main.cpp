#include <Arduino.h>
#include <Automaton.h>
#include "setup.h" // Include the setup header

Atm_digital flowmeter; // Button state machine
Atm_button rfid;       // Button state machine

Atm_led valve;
Atm_led led;
Atm_led led_blue;  // Blue LED state machine
Atm_led led_green; // Green LED state machine
Atm_led led_red;   // Red LED state machine

void flow(int idx, int v, int up)
{
  Serial.print("Flow event: ");
  Serial.println(v);
  led.trigger(led.EVT_TOGGLE); // Toggle the LED on flow event
}

void setup()
{
  Serial.begin(9600);
  initialize(rfid, valve, led, led_blue, led_green, led_red);

  flowmeter.begin(FLOWMETER_PIN, 10, false, true)
      .onChange(HIGH, flow)
      .onChange(LOW, flow);

  rfid.onPress([](int idx, int v, int up)
               {
    Serial.println("RFID button pressed");
    valve.trigger(valve.EVT_TOGGLE); });
}

void loop()
{
  automaton.run();
}
