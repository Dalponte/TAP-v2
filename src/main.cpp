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

void startPour(int pulses, const char *id)
{
  pour.start(pulses, id);
  flow_update_timer.start();
}

void handleFlowUpdateTimer(int idx, int v, int up)
{
  pour.updateFlow();
}

void handleMqttMessage(int idx, int v, int up)
{
  Serial.print("MQTT Message received: ");
}

void handlePourDone(int idx, int v, int up)
{
  flow_update_timer.stop();
  Serial.print("Pour completed! Pulses poured: ");
  Serial.print(v);
  Serial.print(", Remaining: ");
  Serial.print(up);
  Serial.print(", ID: ");
  Serial.println(pour.getCurrentId());
}

void handleFlowStatus(int idx, int v, int up)
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

  // Setup pour process
  flowmeter.begin(FLOWMETER_PIN, 1, false, true)
      .onChange(HIGH, flow);

  flow_update_timer.begin(FLOW_UPDATE_INTERVAL_MS)
      .repeat(ATM_COUNTER_OFF)
      .onTimer(handleFlowUpdateTimer);

  pour.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS)
      .onPourDone(handlePourDone)
      .onFlowStatus(handleFlowStatus)
      .trace(Serial);

  // Setup communication
  mqtt.begin(mac, ip, broker, port, "client_id")
      .connect();

  // Temporary button to simulate RFID tag reading
  button.onPress([](int idx, int v, int up)
                 {
                   int pour_pulses = 50;
                   startPour(pour_pulses, "test-tag-123"); });
}

void loop()
{
  automaton.run();
}
