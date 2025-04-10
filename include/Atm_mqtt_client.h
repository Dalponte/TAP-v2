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
  Atm_mqtt_client &begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId);
  Atm_mqtt_client &connect();
  Atm_mqtt_client &disconnect();
  Atm_mqtt_client &onReceive(Machine &machine, int event); // Push connector for onReceive
  Atm_mqtt_client &onReceive(atm_cb_push_t callback, int idx = 0);
  Atm_mqtt_client &onSend(Machine &machine, int event); // Push connector for onSend
  Atm_mqtt_client &onSend(atm_cb_push_t callback, int idx = 0);
  int pull(int id); // Pull connector

  // Publish a message
  Atm_mqtt_client &publish(const char *topic, const char *payload);

  // Trace method
  Atm_mqtt_client &trace(Stream &stream); // Trace method declaration

  // New method for setting up MQTT message handling
  Atm_mqtt_client &setupMessageHandler();

  // New static method for MQTT message callback
  static void onMqttMessage(int messageSize);

private:
  // Event & action handlers
  int event(int id);
  void action(int id);

  // Data members
  EthernetClient _ethClient;
  MqttClient _mqttClient{_ethClient};
  const char *_broker;
  int _port;
  const char *_clientId;
  bool _isConnected;

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
    ON_RECEIVE, // Connector ID for incoming messages
    ON_SEND,    // Connector ID for outgoing messages
    CONN_MAX
  };
  atm_connector connectors[CONN_MAX]; // Array of connectors
};
