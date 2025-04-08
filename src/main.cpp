#include <Arduino.h>
#include <Automaton.h>
#include "setup.h"
#include "Atm_pour.h"
#include "Atm_mqtt_client.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 4, 100);

Atm_led valve;
Atm_led led;
Atm_led led_blue;
Atm_led led_green;
Atm_led led_red;

Atm_digital flowmeter;
Atm_timer flow_timer; // Timer for flow event emissions
Atm_pour pour;

Atm_button button;
Atm_mqtt_client mqtt;

const char broker[] = "192.168.4.2"; // MQTT broker address
int port = 1883;                     // MQTT broker port

void flow(int idx, int v, int up)
{
  led.trigger(led.EVT_TOGGLE);
  pour.flow();
}

void onCounterReached(int idx, int v, int up)
{
  Serial.print("Counter reached: ");
  Serial.println(v);
}

void handleMqttMessage(int idx, int v, int up)
{
  Serial.print("MQTT Message received: ");
}

// Callback handler for pour done event
void pourDoneHandler(int idx, int v, int up)
{
  Serial.print("Pour completed! Pulses poured: ");
  Serial.print(v);
  Serial.print(", Remaining: ");
  Serial.println(up);
}

// Callback handler for remaining change events
void onRemainingChangeHandler(int idx, int v, int up)
{
  Serial.print("Remaining pulses: ");
  Serial.print(v);
  Serial.print(", Poured so far: ");
  Serial.println(up);
}

void setup()
{
  Serial.begin(9600);
  initialize(button, valve, led, led_blue, led_green, led_red);

  // Use the constants from setup.h for the timeouts
  pour.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS)
      .onPourDone(pourDoneHandler)                 // Register pour done handler
      .onRemainingChange(onRemainingChangeHandler) // Register remaining change handler
      .trace(Serial);

  flowmeter.begin(FLOWMETER_PIN, 1, false, true)
      .onChange(HIGH, flow);

  mqtt.begin(mac, ip, broker, port, "client_id")
      .connect();

  button.onPress([](int idx, int v, int up)
                 {
                   int pour_pulses = 50;
                   pour.start(pour_pulses); });
}

void loop()
{
  automaton.run();
}
