#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "MqttService.h"
#include "TapService.h"
#include "LedService.h"
#include "Atm_tap.h"

class Controller
{
public:
    static Controller &getInstance(MqttService &mqtt, TapService &tap, LedService &led);

    static void setup();

    static void handleMqttMessage(const char *topic, const char *message);
    static void handlePourStart(const char *message);
    static void handlePourDone(const char *id, int pulses, int remaining);
    static void handleFlowStatus(const char *id, int flowRate, int totalPulses);
    static void handlePourStarted(const char *id, int pulses);

    // Add the state change publish handler
    static void publishTapStateChanged(int idx, int state, int up);

    Atm_tap _tap;

private:
    Controller(MqttService &mqtt, TapService &tap, LedService &led);

    // No copy or assignment
    Controller(const Controller &) = delete;
    Controller &operator=(const Controller &) = delete;

    static MqttService *mqttService;
    static TapService *tapService;
    static LedService *ledService;
};

#endif // CONTROLLER_H
