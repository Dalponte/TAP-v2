#include "Atm_mqtt_client.h"

Atm_mqtt_client &Atm_mqtt_client::begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId)
{
    // clang-format off
    const static state_t state_table[] PROGMEM = {
        /*                ON_ENTER            ON_LOOP  ON_EXIT  EVT_CONNECT  EVT_DISCONNECT  ELSE */
        /* DISCONNECTED */ ENT_DISCONNECTED, -1,      -1,      CONNECTED,   -1,             -1,
        /* CONNECTED */    ENT_CONNECTED,    -1,      -1,      -1,          DISCONNECTED,   -1,
    };
    // clang-format on
    Machine::begin(state_table, ELSE);

    // Initialize Ethernet
    Ethernet.begin(mac, ip);
    delay(1000); // Allow Ethernet to initialize

    // Store connection parameters
    _broker = broker;
    _port = port;
    _clientId = clientId;
    _isConnected = false;

    return *this;
}

int Atm_mqtt_client::event(int id)
{
    switch (id)
    {
    case EVT_CONNECT:
        return !_isConnected && _mqttClient.connect(_broker, _port);
    }
    return 0;
}

void Atm_mqtt_client::action(int id)
{
    switch (id)
    {
    case ENT_CONNECTED:
        _isConnected = true;
        Serial.println("MQTT Connected");
        return;

    case ENT_DISCONNECTED:
        _isConnected = false;
        _mqttClient.stop(); // Disconnect from the MQTT broker
        Serial.println("MQTT Disconnected");
        return;
    }
}

Atm_mqtt_client &Atm_mqtt_client::connect()
{
    trigger(EVT_CONNECT);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::disconnect()
{
    if (_isConnected)
    {
        trigger(EVT_DISCONNECT);
    }
    return *this;
}
