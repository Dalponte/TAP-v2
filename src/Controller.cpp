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

    // Initialize valve
    _valve.begin(VALVE_PIN);
    _valve.trigger(_valve.EVT_OFF);

    // Setup flow meter
    _flowmeter.begin(FLOWMETER_PIN, 1, false, true)
        .onChange(HIGH, handleFlow);

    // Setup flow update timer
    _flow_update_timer.begin(FLOW_UPDATE_INTERVAL_MS)
        .repeat(ATM_COUNTER_OFF)
        .onTimer(handleFlowUpdateTimer);

    _tap.begin(*ledService) // Pass the LedService instance to Atm_tap's begin method
        .trace(Serial)
        .onStateChange(Controller::publishTapStateChanged)
        .onDone(Controller::onTapDone)
        .onFlowStatus(Controller::onFlowStatus); // Register flow status handler

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

void Controller::onConnected(int idx, int v, int up)
{
    _instance->_mqttClient.onMessage(handleMqttMessage);
    _instance->_mqttClient.subscribe("tap/command");
    _instance->_tap.trigger(Atm_tap::EVT_CONNECTED);
}

void Controller::onDisconnected(int idx, int v, int up)
{
    _instance->_tap.trigger(Atm_tap::EVT_DISCONNECT);
}

void Controller::publish(const char *topic, const char *payload)
{
    _mqtt.publish(topic, payload);
}

void Controller::handleError(const char *errorMessage)
{
    Serial.print("Error: ");
    Serial.println(errorMessage);
    _instance->publish("tap/error", errorMessage);
}

void Controller::handleMqttMessage(int messageSize)
{
    if (!_instance)
        return;

    String topicStr = _instance->_mqttClient.messageTopic();

    // Clear previous message buffer
    memset(_instance->_messageBuffer, 0, sizeof(_instance->_messageBuffer));

    size_t index = 0;
    while (_instance->_mqttClient.available() && index < sizeof(_instance->_messageBuffer) - 1)
    {
        _instance->_messageBuffer[index++] = (char)_instance->_mqttClient.read();
    }

    Command command = MessageUtils::parseCommand(_instance->_messageBuffer, index);

    if (command.errorCode == MSG_SUCCESS)
    {
        processCommand(command);
    }
    else
    {
        handleError(MessageUtils::getErrorDescription(command.errorCode));
    }
    Serial.print(topicStr);
    Serial.print(" - size: ");
    Serial.println(messageSize);
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

// Update onTapDone to control the valve
void Controller::onTapDone(int idx, int poured, int remaining)
{
    // Close valve when pour is done
    if (_instance)
    {
        _instance->_flow_update_timer.stop();
        _instance->_valve.trigger(_instance->_valve.EVT_OFF); // Close valve
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "{\"data\":[%d,%d,%d]}", _instance->_config.tapId, poured, remaining);
    _instance->publish("tap/done", msg);
}

// Rename processJsonCommand to processCommand for clarity
void Controller::processCommand(const Command &command) // Renamed parameter from processJsonCommand
{
    // Check if the command is for this specific tap instance
    if (command.tapId != _instance->_config.tapId)
    {
        Serial.print("Command ignored: Target Tap ID (");
        Serial.print(command.tapId);
        Serial.print(") does not match this Tap ID (");
        Serial.print(_instance->_config.tapId);
        Serial.println(").");
        return; // Ignore commands not meant for this tap
    }

    // Debug output
    Serial.println("Processing Command:");
    Serial.print("  Tap ID: ");
    Serial.println(command.tapId);
    Serial.print("  Command Type: ");
    Serial.println(command.commandType);
    Serial.print("  Pulses: ");
    Serial.println(command.param);

    // Handle different commands
    switch (command.commandType)
    {
    case CMD_POUR:
        Serial.println("  Action: Starting pour.");
        _instance->_valve.trigger(_instance->_valve.EVT_ON); // Open valve
        _instance->_flow_update_timer.start(); // Start flow timer
        _instance->_tap.start(command.param, command.tapId);
        break;
    case CMD_CONTINUE:
        Serial.println("  Action: Triggering READY event (Continue).");
        _instance->_tap.trigger(Atm_tap::EVT_READY);
        break; // Added missing break statement

    default:
        Serial.print("  Action: Unknown command type received: ");
        Serial.println(command.commandType);
        handleError(MessageUtils::getErrorDescription(MSG_UNKNOWN_COMMAND));
        break;
    }
}

// Flow meter and valve handlers
void Controller::handleFlow(int idx, int v, int up)
{
    _instance->_tap.flow();
}

void Controller::handleFlowUpdateTimer(int idx, int v, int up)
{
    _instance->_tap.updateFlow();
}

void Controller::onFlowStatus(int idx, int flowRate, int totalPulses)
{
    if (!_instance)
        return;

    // Use binary format for flow status messages
    uint8_t binaryData[6]; // 6 bytes for binary flow data (3 * uint16_t)
    size_t size = MessageUtils::serializeFlowData(
        binaryData, 
        sizeof(binaryData), 
        _instance->_config.tapId, 
        flowRate, 
        totalPulses
    );

    // Publish binary data to MQTT
    if (size > 0) {
        _instance->_mqttClient.beginMessage("tap/flow", size, true, 0); // Use QoS 0, retained = true
        _instance->_mqttClient.write(binaryData, size);
        _instance->_mqttClient.endMessage();
    }
}
