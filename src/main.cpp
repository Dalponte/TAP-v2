#include <Arduino.h>
#include <Automaton.h>
#include "Atm_pour.h"
#include "Atm_mqtt_client.h"
#include "TapService.h"
#include "MqttService.h"
#include "LedService.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 4, 100);

#define BUTTON_PIN 23 // Button to trigger the pour

Atm_button button;

// Use singleton instances for services
TapService &tapService = TapService::getInstance();
MqttService &mqttService = MqttService::getInstance();
LedService &ledService = LedService::getInstance();

const char broker[] = "192.168.4.2"; // MQTT broker address
int port = 1883;                     // MQTT broker port

void handleButtonPress(int idx, int v, int up)
{
  mqttService.publish("tap/out", "Button pressed!");
  ledService.red();
  tapService.startPour(10, "mqtt-test");
}

void onMqttMessageReceived(const char *topic, const char *message)
{
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
    ledService.red();
  }
}

void handlePourStart(const char *message)
{
  Serial.println(message);
  tapService.startPour(50, "mqtt-test");
  ledService.blue();
}

void onPourStarted(const char *id, int pulses)
{
  Serial.print("Pour started - ID: ");
  Serial.print(id);
  Serial.print(" Pulses: ");
  Serial.println(pulses);
  ledService.blue();
}

void handlePourDone(const char *id, int pulses, int remaining)
{
  mqttService.publish("tap/pour", "Pour done!");
  ledService.green();
}

void handleFlowStatus(const char *id, int flowRate, int totalPulses)
{
  mqttService.publish("tap/flow", "Flow status!");
}

void setup()
{
  Serial.begin(9600);

  button.begin(BUTTON_PIN);

  // Initialize TapService with timing parameters
  tapService.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS, FLOW_UPDATE_INTERVAL_MS);

  mqttService.begin(mac, ip, broker, port, "client_id");
  mqttService.onMessage(onMqttMessageReceived);
  mqttService.onPourStart(handlePourStart);

  tapService.onPourDone(handlePourDone);
  tapService.onFlowStatus(handleFlowStatus);
  tapService.onPourStarted(onPourStarted);
  button.onPress(handleButtonPress);
}

void loop()
{
  automaton.run();
}
