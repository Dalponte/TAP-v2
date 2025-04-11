#include "MqttService.h"

// Initialize static instance pointer
MqttService *MqttService::_instance = nullptr;

MqttService::MqttService(Atm_mqtt_client &mqtt)
    : _mqtt(mqtt), _messageCallback(nullptr)
{
    _instance = this;
}

void MqttService::begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId)
{
    // Setup communication
    _mqtt.begin(mac, ip, broker, port, clientId)
        .connect()
        .onReceive(handleMqttMessage)
        .trace(Serial);
}

void MqttService::publish(const char *topic, const char *payload)
{
    _mqtt.publish(topic, payload);
}

void MqttService::publishJson(const char *topic, JsonDocument &doc)
{
    char json[256];
    serializeJson(doc, json);
    _mqtt.publish(topic, json);
}

void MqttService::handleMqttMessage(int idx, int v, int up)
{
    Serial.print(">>>up: ");
    Serial.println(up);

    if (_instance && _instance->_messageCallback)
    {
        // Cast the integer back to a char pointer to get the message
        const char *message = (const char *)v;
        _instance->_messageCallback("tap/in", message);
    }
}

void MqttService::onMessage(MessageCallback callback)
{
    _messageCallback = callback;
}

PourRequest MqttService::parsePourRequest(const char *message)
{
    PourRequest result = {nullptr, 0, false, nullptr};
    static char errorBuffer[80]; // Buffer for storing error messages

    JsonDocument mqttDoc;
    JsonDocument dataDoc;

    DeserializationError mqttError = deserializeJson(mqttDoc, message);
    if (mqttError)
    {
        snprintf(errorBuffer, sizeof(errorBuffer), "Error parsing MQTT JSON: %s", mqttError.c_str());
        result.errorMessage = errorBuffer;
        return result;
    }

    const char *data = mqttDoc["data"];
    if (!data)
    {
        result.errorMessage = "Missing 'data' field in MQTT message";
        return result;
    }

    DeserializationError dataError = deserializeJson(dataDoc, data);
    if (dataError)
    {
        snprintf(errorBuffer, sizeof(errorBuffer), "Error parsing data JSON: %s", dataError.c_str());
        result.errorMessage = errorBuffer;
        return result;
    }

    const char *id = dataDoc["id"];
    int remaining = dataDoc["remaining"];

    if (!id)
    {
        result.errorMessage = "Missing 'id' field in data";
        return result;
    }

    if (remaining <= 0)
    {
        result.errorMessage = "Error: remaining value must be a positive number";
        return result;
    }

    result.id = id;
    result.pulses = remaining;
    result.isValid = true;

    return result;
}
