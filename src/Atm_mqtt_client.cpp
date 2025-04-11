#include "Atm_mqtt_client.h"

Atm_mqtt_client *Atm_mqtt_client::_instance = nullptr;

Atm_mqtt_client &Atm_mqtt_client::begin(MqttClient &mqttClient, const char *broker, int port, const char *clientId)
{
    _instance = this;
    // clang-format off
    const static state_t state_table[] PROGMEM = {
        /*                ON_ENTER            ON_LOOP          ON_EXIT  EVT_CONNECT  EVT_DISCONNECT  ELSE */
        /* DISCONNECTED */ ENT_DISCONNECTED, -1,              -1,      CONNECTED,   -1,             -1,
        /* CONNECTED */    ENT_CONNECTED,    LP_CONNECTED,    -1,      -1,          DISCONNECTED,   -1,
    };
    // clang-format on
    Machine::begin(state_table, ELSE);

    _mqttClient = &mqttClient;
    _broker = broker;
    _port = port;
    _clientId = clientId;
    _isConnected = false;
    _lastConnectionCheck = millis();

    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::publish(const char *topic, const char *payload)
{
    if (_isConnected)
    {
        _mqttClient->beginMessage(topic);
        _mqttClient->print(payload);
        _mqttClient->endMessage();
    }
    return *this;
}

int Atm_mqtt_client::event(int id)
{
    switch (id)
    {
    case EVT_CONNECT:
        return !_isConnected && _mqttClient->connect(_broker, _port);
    }
    return 0;
}

void Atm_mqtt_client::action(int id)
{
    switch (id)
    {
    case ENT_CONNECTED:
        _isConnected = true;

        push(connectors, ON_CONNECTED, 0, 0, 0);
        return;

    case LP_CONNECTED:

        if (millis() - _lastConnectionCheck >= 5000)
        {
            _mqttClient->poll();

            _lastConnectionCheck = millis();
            if (!_mqttClient->connected())
            {
                trigger(EVT_DISCONNECT);
                trigger(EVT_CONNECT);
            }
        }
        return;

    case ENT_DISCONNECTED:
        _isConnected = false;
        // Stop also remove the topic subscriptions and callbacks
        _mqttClient->stop();

        push(connectors, ON_DISCONNECTED, 0, 0, 0);
        // Avoid race condition with reconnecting
        delay(100);
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

Atm_mqtt_client &Atm_mqtt_client::onConnected(Machine &machine, int event)
{
    onPush(connectors, ON_CONNECTED, 0, 1, 1, machine, event);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::onConnected(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_CONNECTED, 0, 1, 1, callback, idx);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::onDisconnected(Machine &machine, int event)
{
    onPush(connectors, ON_DISCONNECTED, 0, 1, 1, machine, event);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::onDisconnected(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_DISCONNECTED, 0, 1, 1, callback, idx);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "MQTT_CLIENT\0EVT_CONNECT\0EVT_DISCONNECT\0ELSE\0"
                      "DISCONNECTED\0CONNECTED");
    return *this;
}
