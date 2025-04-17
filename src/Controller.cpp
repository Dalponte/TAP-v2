#include "Controller.h"
#include <Arduino.h>
#include "Atm_tap.h"

MqttService *Controller::mqttService = nullptr;
TapService *Controller::tapService = nullptr;
LedService *Controller::ledService = nullptr;

Controller &Controller::getInstance(MqttService &mqtt, TapService &tap, LedService &led)
{
    static Controller instance(mqtt, tap, led);
    return instance;
}

Controller::Controller(MqttService &mqtt, TapService &tap, LedService &led)
{
    mqttService = &mqtt;
    tapService = &tap;
    ledService = &led;
    _tap.begin().trace(Serial).onStateChange(Controller::publishTapStateChanged); // Register handler
}

void Controller::setup()
{
    mqttService->onMessage(Controller::handleMqttMessage);
    mqttService->onPourStart(Controller::handlePourStart);
    tapService->onPourDone(Controller::handlePourDone);
    tapService->onFlowStatus(Controller::handleFlowStatus);
    tapService->onPourStarted(Controller::handlePourStarted);
}

void Controller::handleMqttMessage(const char *topic, const char *message)
{
    PourRequest request = MqttService::parsePourRequest(message);

    if (request.isValid)
    {
        Serial.print("ID: ");
        Serial.print(request.id);
        Serial.print(", Remaining: ");
        Serial.println(request.pulses);

        tapService->startPour(request.pulses, request.id);
    }
    else
    {
        Serial.println(request.errorMessage);
        if (ledService)
            ledService->red();
    }
}

void Controller::handlePourStart(const char *message)
{
    tapService->startPour(50, "mqtt-test");
    if (ledService)
        ledService->blue();
}

void Controller::handlePourDone(const char *id, int pulses, int remaining)
{
    mqttService->publish("tap/pour", "Pour done!");
    if (ledService)
        ledService->green();
}

void Controller::handleFlowStatus(const char *id, int flowRate, int totalPulses)
{
    mqttService->publish("tap/flow", "Flow status!");
}

void Controller::handlePourStarted(const char *id, int pulses)
{
    if (ledService)
        ledService->blue();
}

void Controller::publishTapStateChanged(int idx, int state, int up)
{
    if (!mqttService)
        return;
    const char *stateStr = nullptr;
    switch (state)
    {
    case Atm_tap::INITIALIZING:
        stateStr = "INITIALIZING";
        break;
    case Atm_tap::READY:
        stateStr = "READY";
        break;
    case Atm_tap::POURING:
        stateStr = "POURING";
        break;
    case Atm_tap::DONE:
        stateStr = "DONE";
        break;
    case Atm_tap::DISCONNECTED:
        stateStr = "DISCONNECTED";
        break;
    default:
        stateStr = "UNKNOWN";
        break;
    }
    char msg[64];
    snprintf(msg, sizeof(msg), "{\"state\":\"%s\"}", stateStr);
    mqttService->publish("tap/state", msg);
}
