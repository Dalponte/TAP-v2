#include <Arduino.h>
#include <Automaton.h>
#include "setup.h"
#include "Atm_pour.h"
#include "Atm_mqtt_client.h"
#include "TapService.h"
#include "MqttService.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 4, 100);

Atm_led valve;
Atm_led led;
Atm_led led_blue;
Atm_led led_green;
Atm_led led_red;

Atm_button button;

// Use singleton instance for both services
TapService &tapService = TapService::getInstance();
MqttService &mqttService = MqttService::getInstance();

const char broker[] = "192.168.4.2"; // MQTT broker address
int port = 1883;                     // MQTT broker port

void handleButtonPress(int idx, int v, int up)
{
  mqttService.publish("tap/out", "Button pressed!");
  led_blue.trigger(led_blue.EVT_OFF);
}

void onMqttMessageReceived(const char *topic, const char *message)
{
  led_green.trigger(led_green.EVT_ON);

  PourRequest request = MqttService::parsePourRequest(message);

  if (request.isValid)
  {
    Serial.print("ID: ");
    Serial.print(request.id);
    Serial.print(", Remaining: ");
    Serial.println(request.pulses);

    tapService.startPour(request.pulses, request.id);
  }
  else
  {
    Serial.println(request.errorMessage);
    led_red.trigger(led_red.EVT_ON);
    delay(3000);
    led_red.trigger(led_red.EVT_OFF);
  }

  led_green.trigger(led_green.EVT_OFF);
}

void handlePourStart(const char *message)
{
  Serial.println(message);

  // @TODO For testing, start a simple pour with fixed values
  // This doesn't parse the message yet, just triggers the pour
  tapService.startPour(10, "mqtt-test");

  led_blue.trigger(led_blue.EVT_ON);
}

// Callbacks for TapService events
void handlePourDone(const char *id, int pulses, int remaining)
{
  mqttService.publish("tap/pour", "Pour done!");
}

void handleFlowStatus(const char *id, int flowRate, int totalPulses)
{
  mqttService.publish("tap/flow", "Flow status!");
}

void setup()
{
  Serial.begin(9600);
  initialize(button, valve, led, led_blue, led_green, led_red);

  tapService.init(valve, led);

  mqttService.begin(mac, ip, broker, port, "client_id");
  mqttService.onMessage(onMqttMessageReceived);
  mqttService.onPourStart(handlePourStart);

  tapService.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS, FLOW_UPDATE_INTERVAL_MS);
  tapService.onPourDone(handlePourDone);
  tapService.onFlowStatus(handleFlowStatus);

  // Temporary button to simulate RFID tag reading and publish a message
  button.onPress(handleButtonPress);
}

void loop()
{
  automaton.run();
}
