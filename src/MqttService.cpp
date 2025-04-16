#include "MqttService.h"

// Initialize static instance pointer
MqttService *MqttService::_instance = nullptr;

MqttService::MqttService()
    : _messageCallback(nullptr), _pourStartCallback(nullptr)
{
    memset(_messageBuffer, 0, sizeof(_messageBuffer));
}

void MqttService::begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId)
{
    Ethernet.begin(mac, ip);
    delay(1500); // Allow Ethernet to initialize

    _mqtt.begin(_mqttClient, broker, port, clientId)
        .onConnected(onConnected)
        .onDisconnected(onDisconnected)
        .trace(Serial)
        .connect();
}

void MqttService::onConnected(int idx, int v, int up)
{
    _instance->_mqttClient.onMessage(handleMqttMessage);
    _instance->_mqttClient.subscribe("tap/in");
}

void MqttService::onDisconnected(int idx, int v, int up)
{
}

void MqttService::publish(const char *topic, const char *payload)
{
    _mqtt.publish(topic, payload);
}

void MqttService::handleMqttMessage(int messageSize)
{
    if (_instance)
    {
        // Get topic as String and then convert to const char*
        String topicStr = _instance->_mqttClient.messageTopic();
        Serial.print("Topic: ");
        Serial.print(topicStr);
        Serial.print(", Length: ");
        Serial.println(messageSize);

        // Clear previous message
        memset(_instance->_messageBuffer, 0, sizeof(_instance->_messageBuffer));

        // Read the message into our buffer
        size_t index = 0;
        while (_instance->_mqttClient.available() && index < sizeof(_instance->_messageBuffer) - 1)
        {
            _instance->_messageBuffer[index++] = (char)_instance->_mqttClient.read();
        }
        _instance->_messageBuffer[index] = '\0';

        Serial.print("Content: ");
        Serial.println(_instance->_messageBuffer);

        // Call user message callback if registered
        if (_instance->_messageCallback)
        {
            _instance->_messageCallback(topicStr.c_str(), _instance->_messageBuffer);
        }

        // If this is a pour request topic, call the pour start callback
        if (topicStr == "tap/in" && _instance->_pourStartCallback)
        {
            _instance->_pourStartCallback(_instance->_messageBuffer);
        }
    }
}

void MqttService::processMessage()
{
    Serial.println("processMessage() is deprecated, message already processed by handler");
}

void MqttService::onMessage(MessageCallback callback)
{
    _messageCallback = callback;
}

void MqttService::onPourStart(PourStartCallback callback)
{
    _pourStartCallback = callback;
}

PourRequest MqttService::parsePourRequest(const char *message)
{
    PourRequest request;

    // Simply print the message without parsing
    Serial.println("Pour request received:");
    Serial.println(message);

    // Set default values for the request
    request.isValid = false;
    request.errorMessage = "Parse not implemented";
    request.id = "";
    request.pulses = 0;

    return request;
}
