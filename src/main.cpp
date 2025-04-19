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
IPAddress ip(192, 168, 4, 99);
const char broker[] = "192.168.4.2";
int port = 1883;

const TapConfig tapConfig = {
    .tapId = 99,
    .tapName = "01"};

// TapService &tapService = TapService::getInstance(5000, 1000);
LedService &ledService = LedService::getInstance();
Controller &controller = Controller::initInstance(ledService, tapConfig);

void handleButtonPress(int idx, int v, int up)
{
  char payload[64];
  snprintf(payload, sizeof(payload), "{\"data\":[%d, \"%s\"]}", tapConfig.tapId, "tag-id");
  controller.publish("tap/input", payload);
  // ledService.red();
  // tapService.startPour(10, "mqtt-test"); // deprecated
}

void setup()
{
  Serial.begin(9600);

  controller.begin(mac, ip, broker, port, "client_id");

  button.begin(BUTTON_PIN);
  button.onPress(handleButtonPress);
}

void loop()
{
  automaton.run();
}
