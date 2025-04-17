#include <Arduino.h>
#include <Automaton.h>
#include "Atm_pour.h"
#include "Atm_mqtt_client.h"
#include "TapService.h"
#include "LedService.h"
#include "Controller.h"

#define BUTTON_PIN 23
Atm_button button;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 4, 100);
const char broker[] = "192.168.4.2"; // MQTT broker address
int port = 1883;                     // MQTT broker port

// Hardcoded tap configuration
const TapConfig tapConfig = {
    .tapId = 1, // Integer ID for the tap
    .tapName = "01"};

TapService &tapService = TapService::getInstance(5000, 1000);
LedService &ledService = LedService::getInstance();
// Initialize the controller singleton with LED service and config
Controller &controller = Controller::initInstance(ledService, tapConfig);

void handleButtonPress(int idx, int v, int up)
{
  controller.publish("tap/out", "Button pressed!");
  ledService.red();
  tapService.startPour(10, "mqtt-test");
}

void setup()
{
  Serial.begin(9600);

  // Initialize MQTT directly in the Controller
  controller.begin(mac, ip, broker, port, "client_id");

  Controller::setup();

  button.begin(BUTTON_PIN); // Temporary button to simulate pour requests
  button.onPress(handleButtonPress);
}

void loop()
{
  automaton.run();
}
