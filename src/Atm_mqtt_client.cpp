#include "Atm_mqtt_client.h"

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

    Ethernet.begin(mac, ip);
    delay(1000); // Allow Ethernet to initialize

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
        _mqttClient.beginMessage(topic);
        _mqttClient.print(payload);
        _mqttClient.endMessage();

        // Push the message through the ON_SEND connector
        push(connectors, ON_SEND, 0, 0, 0);
    }
    return *this;
}

// Add global reference to the callback function
static atm_cb_push_t mqttMessageCallback = nullptr;
static int mqttCallbackIdx = 0;

// Add a static buffer to hold the message between calls
static char persistentMessageBuffer[256];

void Atm_mqtt_client::onMqttMessage(int messageSize)
{
    if (!_instance)
        return;

    Serial.print("Atm_mqtt_client: MQTT Message received. Topic: ");
    Serial.print(_instance->_mqttClient.messageTopic());
    Serial.print(", Length: ");
    Serial.println(messageSize);

    // Clear buffer before reading new message
    memset(persistentMessageBuffer, 0, sizeof(persistentMessageBuffer));
    size_t index = 0;

    while (_instance->_mqttClient.available() && index < sizeof(persistentMessageBuffer) - 1)
    {
        persistentMessageBuffer[index++] = (char)_instance->_mqttClient.read();
    }
    persistentMessageBuffer[index] = '\0';

    Serial.print("Content: ");
    Serial.println(persistentMessageBuffer);

    // Call the callback directly if it exists
    if (mqttMessageCallback != nullptr)
    {
        Serial.println("Calling callback directly");
        mqttMessageCallback(mqttCallbackIdx, (int)persistentMessageBuffer, messageSize);
    }
    else
    {
        Serial.println("ERROR: No callback registered");
    }
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

        // Check connection every 5 seconds
        if (millis() - _lastConnectionCheck >= 5000)
        {
            _lastConnectionCheck = millis();
            if (!_mqttClient.connected())
            {
                trigger(EVT_DISCONNECT);
                trigger(EVT_CONNECT);
            }
        }
        return;

    case ENT_DISCONNECTED:
        Serial.println("MQTT Disconnected");
        _isConnected = false;
        _mqttClient.stop();
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
    // Store callback for direct use
    mqttMessageCallback = callback;
    mqttCallbackIdx = idx;

    // Also register with connector system as before
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
        return _isConnected ? 1 : 0;
    }
    return 0;
}

Atm_mqtt_client &Atm_mqtt_client::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "MQTT_CLIENT\0EVT_CONNECT\0EVT_DISCONNECT\0ELSE\0"
                      "DISCONNECTED\0CONNECTED");
    return *this;
}
