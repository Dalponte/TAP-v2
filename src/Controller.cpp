#include "Controller.h"
#include <Arduino.h>

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
    Serial.println(message);
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
    Serial.print("Pour started - ID: ");
    Serial.print(id);
    Serial.print(" Pulses: ");
    Serial.println(pulses);
    if (ledService)
        ledService->blue();
}
