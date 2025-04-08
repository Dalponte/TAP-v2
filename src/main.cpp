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
Atm_timer flow_update_timer;
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

// Flow timer callback to update remaining values periodically
void onFlowTimer(int idx, int v, int up)
{
  pour.updateFlow();
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

void pourDoneHandler(int idx, int v, int up)
{
  flow_update_timer.stop();
  Serial.print("Pour completed! Pulses poured: ");
  Serial.print(v);
  Serial.print(", Remaining: ");
  Serial.println(up);
}

void onFlowStatusHandler(int idx, int v, int up)
{
  Serial.println();
  Serial.print(up);
  Serial.print(" |  ");
  Serial.print(v);
}

void setup()
{
  Serial.begin(9600);
  initialize(button, valve, led, led_blue, led_green, led_red);

  pour.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS)
      .onPourDone(pourDoneHandler)       // Register pour done handler
      .onFlowStatus(onFlowStatusHandler) // Register flow status handler with renamed method
      .trace(Serial);

  flowmeter.begin(FLOWMETER_PIN, 1, false, true)
      .onChange(HIGH, flow);

  mqtt.begin(mac, ip, broker, port, "client_id")
      .connect();

  button.onPress([](int idx, int v, int up)
                 {
                   int pour_pulses = 50;
                   pour.start(pour_pulses);
                   flow_update_timer.start(); // Start flow timer when pouring begins
                 });

  flow_update_timer.begin(FLOW_UPDATE_INTERVAL_MS)
      .repeat(ATM_COUNTER_OFF) // Run indefinitely
      .onTimer(onFlowTimer);   // Use our callback to check pour status
}

void loop()
{
  automaton.run();
}
