#pragma once

#include <Automaton.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoMqttClient.h>

class Atm_mqtt_client : public Machine
{
public:
  enum
  {
    DISCONNECTED,
    CONNECTED
  }; // States
  enum
  {
    EVT_CONNECT,
    EVT_DISCONNECT,
    ELSE
  }; // Events

  Atm_mqtt_client(void) : Machine() {};

  // Methods
  Atm_mqtt_client &begin(MqttClient &mqttClient, const char *broker, int port, const char *clientId);
  Atm_mqtt_client &connect();
  Atm_mqtt_client &disconnect();
  Atm_mqtt_client &onReceive(Machine &machine, int event);
  Atm_mqtt_client &onReceive(atm_cb_push_t callback, int idx = 0);
  Atm_mqtt_client &onSend(Machine &machine, int event);
  Atm_mqtt_client &onSend(atm_cb_push_t callback, int idx = 0);
  Atm_mqtt_client &publish(const char *topic, const char *payload);
  Atm_mqtt_client &trace(Stream &stream);
  Atm_mqtt_client &setupMessageHandler();
  static void onMqttMessage(int messageSize);
  int pull(int id); // Pull connector

private:
  // Reference to the current instance for the static callback
  static Atm_mqtt_client *_instance;

  // Internal events/actions
  enum
  {
    ENT_CONNECTED,
    ENT_DISCONNECTED,
    LP_CONNECTED
  };

  // Connectors
  enum
  {
    ON_RECEIVE,
    ON_SEND,
    CONN_MAX
  };

  int event(int id);
  void action(int id);

  // Data members
  MqttClient *_mqttClient; // Pointer instead of object
  const char *_broker;
  int _port;
  const char *_clientId;
  bool _isConnected;

  unsigned long _lastConnectionCheck = 0;
  atm_connector connectors[CONN_MAX];
};
