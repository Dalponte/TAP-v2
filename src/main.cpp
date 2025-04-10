#include <Arduino.h>
#include <Automaton.h>
#include <ArduinoJson.h>
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

void handleButtonPress(int idx, int v, int up)
{
  JsonDocument doc;
  doc["id"] = "button-press";

  char json[64];
  serializeJson(doc, json);
  mqtt.publish("tap/out", json);
  led_blue.trigger(led.EVT_OFF);
}

void handleMqttMessage(int idx, int v, int up)
{
  led_green.trigger(led.EVT_ON);
  JsonDocument mqttDoc;
  JsonDocument dataDoc;

  // Cast the integer back to a char pointer to get the message
  const char *message = (const char *)v;
  DeserializationError mqttError = deserializeJson(mqttDoc, message);

  if (!mqttError)
  {
    const char *data = mqttDoc["data"];

    DeserializationError dataError = deserializeJson(dataDoc, data);

    if (!dataError)
    {
      const char *id = dataDoc["id"];
      int remaining = dataDoc["remaining"];
      Serial.print("ID: ");
      Serial.print(id);
      Serial.print(", Remaining: ");
      Serial.println(remaining);

      if (remaining > 0)
      {
        tapService.startPour(remaining, id);
      }
      else
      {
        Serial.println("Error: remaining value must be a positive number");
        led_red.trigger(led.EVT_ON);
        delay(3000);
        led_red.trigger(led.EVT_OFF);
      }
    }
    else
    {
      Serial.print("Error parsing data JSON: ");
      Serial.println(dataError.c_str());
      led_red.trigger(led.EVT_ON);
      delay(3000);
      led_red.trigger(led.EVT_OFF);
    }
  }
  else
  {
    Serial.print("Error parsing MQTT JSON: ");
    Serial.println(mqttError.c_str());
    led_red.trigger(led.EVT_ON);
    delay(3000);
    led_red.trigger(led.EVT_OFF);
  }
  led_green.trigger(led.EVT_OFF);
}

void setup()
{
  Serial.begin(9600);
  initialize(button, valve, led, led_blue, led_green, led_red);

  // Setup communication
  mqtt.begin(mac, ip, broker, port, "client_id")
      .connect()
      .onReceive(handleMqttMessage)
      .trace(Serial);

  // Initialize tap service
  tapService.begin(INITIAL_TIMEOUT_MS, CONTINUE_TIMEOUT_MS, FLOW_UPDATE_INTERVAL_MS);

  // Temporary button to simulate RFID tag reading and publish a message
  button.onPress(handleButtonPress);
}

void loop()
{
  automaton.run();
}
