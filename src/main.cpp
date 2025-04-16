#include <Arduino.h>
#include <Automaton.h>
#include "Atm_pour.h"
#include "Atm_mqtt_client.h"
#include "TapService.h"
#include "MqttService.h"
#include "LedService.h"
#include "Controller.h"

#define BUTTON_PIN 23
Atm_button button;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 4, 100);
const char broker[] = "192.168.4.2"; // MQTT broker address
int port = 1883;                     // MQTT broker port

TapService &tapService = TapService::getInstance(5000, 1000);
MqttService &mqttService = MqttService::getInstance();
LedService &ledService = LedService::getInstance();
Controller &controller = Controller::getInstance(mqttService, tapService, ledService);

void handleButtonPress(int idx, int v, int up)
{
  mqttService.publish("tap/out", "Button pressed!");
  ledService.red();
  tapService.startPour(10, "mqtt-test");
}

void setup()
{
  Serial.begin(9600);

  mqttService.begin(mac, ip, broker, port, "client_id");

  Controller::setup();

  button.begin(BUTTON_PIN); // Temporary button to simulate pour requests
  button.onPress(handleButtonPress);
}

void loop()
{
  automaton.run();
}
