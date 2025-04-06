#include <Arduino.h>
#include <Automaton.h>

#define PIN_LED 13
#define PIN_BLUE 44
#define PIN_GREEN 45
#define PIN_RED 46
#define FLOWMETER_BUTTON 21 // Flowmeter button simulation pin
#define RFID_BUTTON 23      // RFID button simulation pin

Atm_button flow; // Button state machine
Atm_button rfid; // Button state machine

Atm_led led;
Atm_led led_blue;  // Blue LED state machine
Atm_led led_green; // Green LED state machine
Atm_led led_red;   // Red LED state machine

void initializeLEDs()
{
  led.begin(PIN_LED);
  led_blue.begin(PIN_BLUE);
  led_green.begin(PIN_GREEN);
  led_red.begin(PIN_RED);

  led.trigger(led.EVT_ON);
  led_blue.trigger(led.EVT_ON);
  led_green.trigger(led.EVT_ON);
  led_red.trigger(led.EVT_ON);
}

void initializeButtons()
{
  flow.begin(FLOWMETER_BUTTON);
  flow.onPress([](int idx, int v, int up)
               {
                   Serial.println("Flowmeter button pressed");
                   led_green.trigger(led.EVT_TOGGLE); });

  rfid.begin(RFID_BUTTON);
  rfid.onPress([](int idx, int v, int up)
               {
                   Serial.println("RFID button pressed");
                   led_blue.trigger(led.EVT_TOGGLE); });
}

void setup()
{
  Serial.begin(9600);
  initializeLEDs();
  initializeButtons();
}

void loop()
{
  automaton.run();
}
