#include "Atm_mqtt_client.h"

// Initialize the static pointer to null
Atm_mqtt_client *Atm_mqtt_client::_instance = nullptr;

Atm_mqtt_client &Atm_mqtt_client::begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId)
{
    // Set the static instance pointer to this instance
    _instance = this;

    // clang-format off
    const static state_t state_table[] PROGMEM = {
        /*                ON_ENTER            ON_LOOP          ON_EXIT  EVT_CONNECT  EVT_DISCONNECT  ELSE */
        /* DISCONNECTED */ ENT_DISCONNECTED, -1,              -1,      CONNECTED,   -1,             -1,
        /* CONNECTED */    ENT_CONNECTED,    LP_CONNECTED,    -1,      -1,          DISCONNECTED,   -1,
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

// Static callback function for MQTT messages
void Atm_mqtt_client::onMqttMessage(int messageSize)
{
    // Make sure we have an instance
    if (!_instance)
    {
        return;
    }

    // Print message details
    Serial.print("MQTT Message received. Topic: ");
    Serial.print(_instance->_mqttClient.messageTopic());
    Serial.print(", Length: ");
    Serial.println(messageSize);

    // Print message content
    Serial.print("Content: ");
    // Read the message byte by byte
    while (_instance->_mqttClient.available())
    {
        Serial.print((char)_instance->_mqttClient.read());
    }
    Serial.println();

    // Trigger the ON_RECEIVE connector
    _instance->push(_instance->connectors, ON_RECEIVE, 0, 0, 0);
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
        Serial.println("MQTT Connected");
        _isConnected = true;

        _mqttClient.onMessage(onMqttMessage);
        _mqttClient.subscribe("tap/in");
        return;

    case LP_CONNECTED:
        // Keep the connection alive
        _mqttClient.poll();
        return;

    case ENT_DISCONNECTED:
        Serial.println("MQTT Disconnected");
        _isConnected = false;
        _mqttClient.stop(); // Disconnect from the MQTT broker
        _mqttClient.unsubscribe("tap/in");
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

Atm_mqtt_client &Atm_mqtt_client::onReceive(Machine &machine, int event)
{
    onPush(connectors, ON_RECEIVE, 0, 1, 1, machine, event);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::onReceive(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_RECEIVE, 0, 1, 1, callback, idx);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::onSend(Machine &machine, int event)
{
    onPush(connectors, ON_SEND, 0, 1, 1, machine, event);
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::onSend(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_SEND, 0, 1, 1, callback, idx);
    return *this;
}

int Atm_mqtt_client::pull(int id)
{
    switch (id)
    {
    case ON_SEND:
        return _isConnected ? 1 : 0; // Return 1 if connected, 0 otherwise
    }
    return 0;
}

Atm_mqtt_client &Atm_mqtt_client::publish(const char *topic, const char *payload)
{
    if (_isConnected)
    {
        _mqttClient.beginMessage(topic);
        _mqttClient.print(payload);
        _mqttClient.endMessage();

        Serial.print("Published message to ");
        Serial.print(topic);
        Serial.print(": ");
        Serial.println(payload);

        // Push the message through the ON_SEND connector
        push(connectors, ON_SEND, 0, 0, 0);
    }
    return *this;
}

Atm_mqtt_client &Atm_mqtt_client::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "MQTT_CLIENT\0EVT_CONNECT\0EVT_DISCONNECT\0ELSE\0"
                      "DISCONNECTED\0CONNECTED");
    return *this;
}
