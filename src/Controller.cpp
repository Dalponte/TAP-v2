#include "Controller.h"
#include <Arduino.h>
#include "Atm_tap.h"
#include "MessageUtils.h"
#include <ArduinoJson.h>

LedService *Controller::ledService = nullptr;
Controller *Controller::_instance = nullptr;

Controller &Controller::initInstance(LedService &led, const TapConfig &config)
{
    if (_instance == nullptr)
    {
        _instance = new Controller(led, config);
    }
    return *_instance;
}

Controller &Controller::getInstance()
{
    if (_instance == nullptr)
    {
        Serial.println("ERROR: Controller not initialized. Call initInstance first.");
        while (true)
        {
            delay(1000); // Halt to prevent further execution with uninitialized controller
        }
    }
    return *_instance;
}

Controller::Controller(LedService &led, const TapConfig &config)
    : _config(config)
{
    ledService = &led; // Assign the provided LedService instance

    ledService->setBlue(true);
    ledService->setRed(true);
    ledService->setGreen(true);

    _tap.begin()
        .trace(Serial)
        .onStateChange(Controller::publishTapStateChanged)
        .onInitializing(Controller::onTapInitializing)
        .onReady(Controller::onTapReady)
        .onPouring(Controller::onTapPouring)
        .onDone(Controller::onTapDone)
        .onDisconnected(Controller::onTapDisconnected);

    memset(_messageBuffer, 0, sizeof(_messageBuffer));
}

void Controller::begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId)
{
    Ethernet.begin(mac, ip);
    delay(1500); // Allow Ethernet to initialize

    _mqtt.begin(_mqttClient, broker, port, clientId)
        .onConnected(onConnected)
        .onDisconnected(onDisconnected)
        .trace(Serial)
        .connect();
}

void Controller::setup()
{
}

void Controller::onConnected(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_mqttClient.onMessage(handleMqttMessage);
        _instance->_mqttClient.subscribe("tap/command");
        _instance->_tap.trigger(Atm_tap::EVT_CONNECTED);
    }
}

void Controller::onDisconnected(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_tap.trigger(Atm_tap::EVT_DISCONNECT);
    }
}

void Controller::publish(const char *topic, const char *payload)
{
    _mqtt.publish(topic, payload);
}

JsonCommand Controller::parseJsonMessage(const uint8_t *buffer, size_t length)
{
    JsonCommand command = {0, 0, 0, false};
    JsonDocument doc;

    // Ensure null termination for the parser
    char jsonString[length + 1];
    memcpy(jsonString, buffer, length);
    jsonString[length] = '\0';

    DeserializationError error = deserializeJson(doc, jsonString);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        handleError(MessageUtils::getErrorDescription(MSG_JSON_PARSE_ERROR));
        return command;
    }

    // Check if the expected structure is present using the recommended approach
    if (!doc["data"].is<JsonArray>() || doc["data"].size() != 3)
    {
        handleError(MessageUtils::getErrorDescription(MSG_INVALID_FORMAT));
        return command;
    }

    JsonArray data = doc["data"].as<JsonArray>();

    // Extract data, assuming order: [tapId, commandType, pulses]
    command.tapId = data[0].as<uint16_t>();
    command.commandType = data[1].as<uint8_t>();
    command.pulses = data[2].as<uint16_t>();
    command.isValid = true; // Mark as valid if parsing succeeded

    return command;
}

void Controller::handleError(const char *errorMessage)
{
    _instance->publish("tap/error", errorMessage);
}

void Controller::handleMqttMessage(int messageSize)
{
    if (!_instance)
        return;

    String topicStr = _instance->_mqttClient.messageTopic();

    // Clear previous message buffer
    memset(_instance->_messageBuffer, 0, sizeof(_instance->_messageBuffer));

    // Read the message into our buffer
    size_t index = 0;
    while (_instance->_mqttClient.available() && index < sizeof(_instance->_messageBuffer) - 1) // Leave space for null terminator
    {
        _instance->_messageBuffer[index++] = (char)_instance->_mqttClient.read();
    }
    _instance->_messageBuffer[index] = '\0'; // Null-terminate the buffer

    // Print received message for debugging
    Serial.print("Raw MQTT Message: ");
    Serial.println((char *)_instance->_messageBuffer);
    Serial.println("----------");

    // Print basic message info
    Serial.print("MQTT Message - Topic: ");
    Serial.print(topicStr);
    Serial.print(", Size: ");
    Serial.println(index);

    // Check if this is a command message
    if (topicStr == "tap/command")
    {
        // Parse as JSON message
        JsonCommand command = parseJsonMessage(_instance->_messageBuffer, index);

        processJsonCommand(command);
    }
}

void Controller::publishTapStateChanged(int idx, int state, int up)
{
    if (!_instance)
        return;

    const char *stateStr = MessageUtils::stateToString(state);

    // Format simple status message as text
    char msg[64];
    snprintf(msg, sizeof(msg), "{\"state\":\"%s\",\"id\":%d,\"name\":\"%s\"}",
             stateStr,
             _instance->_config.tapId,
             _instance->_config.tapName);

    _instance->publish("tap/state", msg);
}

void Controller::onTapInitializing(int idx, int v, int up)
{
    ledService->red();
}

void Controller::onTapReady(int idx, int v, int up)
{
    ledService->blue();
}

void Controller::onTapPouring(int idx, int v, int up)
{
    ledService->green();
}

void Controller::onTapDone(int idx, int poured, int remaining)
{
    ledService->red();
    char msg[64];
    snprintf(msg, sizeof(msg), "{\"data\":[%d,%d,%d]}", _instance->_config.tapId, poured, remaining);
    _instance->publish("tap/done", msg);
}

void Controller::onTapDisconnected(int idx, int v, int up)
{
    Serial.println("Tap state: DISCONNECTED");
    ledService->setBlue(true);
    ledService->setRed(true);
    ledService->setGreen(true);
}

void Controller::processJsonCommand(const JsonCommand &command)
{
    if (!_instance || !command.isValid)
    {
        handleError("Controller instance is null or command is invalid");
        return;
    }

    if (command.tapId != _instance->_config.tapId)
        return;

    // Debug output
    Serial.print("Tap ID: ");
    Serial.println(command.tapId);
    Serial.print("Command: ");
    Serial.println(command.commandType);
    Serial.print("Pulses: ");
    Serial.println(command.pulses);

    // Handle different commands
    switch (command.commandType)
    {
    case CMD_POUR:
        _instance->_tap.start(command.pulses, command.tapId);
        break;
    case CMD_CONTINUE:
        _instance->_tap.trigger(Atm_tap::EVT_READY);

    default:
        handleError(MessageUtils::getErrorDescription(MSG_UNKNOWN_COMMAND));
        Serial.print("Command type: ");
        Serial.println(command.commandType);
        break;
    }
}
