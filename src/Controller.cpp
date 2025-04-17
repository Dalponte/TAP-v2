#include "Controller.h"
#include <Arduino.h>
#include "Atm_tap.h"
#include "MessageUtils.h"

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
        // In a production environment, you might want to handle this differently
        while (true)
        {
            // Halt to prevent further execution with uninitialized controller
            delay(1000);
        }
    }
    return *_instance;
}

Controller::Controller(LedService &led, const TapConfig &config)
    : _config(config)
{
    ledService = &led;

    // Initialize and configure the tap state machine
    _tap.begin()
        .trace(Serial)
        .onStateChange(Controller::publishTapStateChanged)
        .onInitializing(Controller::onTapInitializing)
        .onReady(Controller::onTapReady)
        .onPouring(Controller::onTapPouring)
        .onDone(Controller::onTapDone)
        .onDisconnected(Controller::onTapDisconnected);

    // Initialize message buffer
    memset(_messageBuffer, 0, sizeof(_messageBuffer));

    Serial.print("Controller initialized with tap ID: ");
    Serial.print(_config.tapId);
    Serial.print(", name: ");
    Serial.println(_config.tapName);
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
    // Nothing to do here since we're handling everything internally now
}

void Controller::onConnected(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_mqttClient.onMessage(handleMqttMessage);
        _instance->_mqttClient.subscribe("tap/in");
        Serial.println("MQTT Connected");

        // Trigger the EVT_CONNECTED event to transition to READY state
        _instance->_tap.trigger(Atm_tap::EVT_CONNECTED);
    }
}

void Controller::onDisconnected(int idx, int v, int up)
{
    Serial.println("MQTT Disconnected");

    if (_instance)
    {
        // Trigger the EVT_DISCONNECT event to transition to DISCONNECTED state
        _instance->_tap.trigger(Atm_tap::EVT_DISCONNECT);
    }
}

void Controller::publish(const char *topic, const char *payload)
{
    _mqtt.publish(topic, payload);
}

bool Controller::parseBinaryMessage(const uint8_t *buffer, size_t length, BinaryMessage &message)
{
    // Delegate to the MessageUtils class
    MessageErrorCode result = MessageUtils::parseBinaryMessage(buffer, length, message);
    return result == MSG_SUCCESS;
}

// Helper method for error handling
void Controller::handleError(const char *errorMessage)
{
    if (errorMessage)
    {
        Serial.print("ERROR: ");
        Serial.println(errorMessage);
    }
}

void Controller::processBinaryMessage(const BinaryMessage &message)
{
    if (!_instance)
    {
        handleError("Controller instance is null");
        return;
    }

    // Debug output
    Serial.print("Protocol version: ");
    Serial.println(message.protocolVersion);
    Serial.print("Tap ID: ");
    Serial.println(message.tapId);
    Serial.print("Command: ");
    Serial.println(message.commandType);
    Serial.print("Param1: ");
    Serial.println(message.param1);
    Serial.print("Param2: ");
    Serial.println(message.param2);
    Serial.print("Param3: ");
    Serial.println(message.param3);

    // Validate message for this tap ID
    MessageErrorCode validationResult = MessageUtils::validateMessage(message, _instance->_config.tapId);

    if (validationResult != MSG_SUCCESS)
    {
        Serial.println(MessageUtils::getErrorDescription(validationResult));
        return;
    }

    Serial.print("Binary message received for this tap (ID: ");
    Serial.print(_instance->_config.tapId);
    Serial.println(")");

    // Handle different commands without try-catch
    switch (message.commandType)
    {
    case CMD_POUR:
        // Trigger pour event on the state machine
        _instance->_tap.trigger(Atm_tap::EVT_POUR);

        if (ledService)
        {
            ledService->blue();
        }
        break;

    case CMD_STOP:
        // Trigger stop command if needed
        Serial.println("Stop command received");
        // Future implementation: _instance->_tap.trigger(Atm_tap::EVT_STOP);
        break;

    case CMD_STATUS:
        Serial.println("Status request received");
        // Just publish current state
        publishTapStateChanged(0, _instance->_tap.state(), 0);
        break;

    default:
        handleError("Unknown command type");
        Serial.print("Command type: ");
        Serial.println(message.commandType);
        break;
    }
}

void Controller::handleMqttMessage(int messageSize)
{
    if (!_instance)
        return;

    // Get topic
    String topicStr = _instance->_mqttClient.messageTopic();

    // Clear previous message
    memset(_instance->_messageBuffer, 0, sizeof(_instance->_messageBuffer));

    // Read the message into our buffer
    size_t index = 0;
    while (_instance->_mqttClient.available() && index < sizeof(_instance->_messageBuffer))
    {
        _instance->_messageBuffer[index++] = (char)_instance->_mqttClient.read();
    }

    // Print basic message info
    Serial.print("MQTT Message - Topic: ");
    Serial.print(topicStr);
    Serial.print(", Size: ");
    Serial.println(index);

    // Check if this is a pour request message
    if (topicStr == "tap/in")
    {
        // Parse as binary message
        BinaryMessage binMsg;
        if (parseBinaryMessage(_instance->_messageBuffer, index, binMsg))
        {
            // Successfully parsed binary message
            Serial.println("Received binary message");
            processBinaryMessage(binMsg);
        }
        else
        {
            Serial.println("Failed to parse binary message");
        }
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
    Serial.println("Tap state: INITIALIZING");
}

void Controller::onTapReady(int idx, int v, int up)
{
    Serial.println("Tap state: READY");
}

void Controller::onTapPouring(int idx, int v, int up)
{
    Serial.println("Tap state: POURING");
}

void Controller::onTapDone(int idx, int v, int up)
{
    Serial.println("Tap state: DONE");
}

void Controller::onTapDisconnected(int idx, int v, int up)
{
    Serial.println("Tap state: DISCONNECTED");
}
