#include <Arduino.h>
#include <Automaton.h>
#include "setup.h"
#include "Atm_pour.h"
#include "Atm_mqtt_client.h"
#include "TapService.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 4, 100);

Atm_led valve;
Atm_led led;
Atm_led led_blue;
Atm_led led_green;
Atm_led led_red;

Atm_button button;
Atm_mqtt_client mqtt;

TapService tapService(valve, led, mqtt);

const char broker[] = "192.168.4.2"; // MQTT broker address
int port = 1883;                     // MQTT broker port

void handleMqttMessage(int idx, int v, int up)
{
  Serial.print(">>>> MQTT Message received: ");
  int pour_pulses = 50;
  tapService.startPour(pour_pulses, "test-tag-123");
}

void setup()
{
  Serial.begin(9600);
  initialize(button, valve, led, led_blue, led_green, led_red);

  // Initialize tap service
  tapService.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS, FLOW_UPDATE_INTERVAL_MS);

  // Setup communication
  mqtt.begin(mac, ip, broker, port, "client_id")
      .onReceive(handleMqttMessage)
      .trace(Serial);

  // Temporary button to simulate RFID tag reading and publish a message
  button.onPress([](int idx, int v, int up)
                 {

                   // Publish a message to the tap/out topic
                   mqtt.publish("tap/out", "Button pressed, pour started"); });
}

void loop()
{
  automaton.run();
}
