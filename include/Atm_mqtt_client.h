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

  // Internal events/actions
  enum
  {
    ENT_CONNECTED,
    ENT_DISCONNECTED
  };
};
