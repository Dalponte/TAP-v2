#pragma once

#include <Automaton.h>
#include "Atm_pour.h"
#include "Atm_mqtt_client.h"

class TapService
{
public:
    TapService(Atm_led &valve, Atm_led &led, Atm_mqtt_client &mqtt);

    // Initialize the service
    void begin(
        int initial_timeout_ms,
        int continue_timeout_ms,
        int flow_update_interval);

    // Start a pour operation
    void startPour(int pulses, const char *id);

    // Get the associated pour machine
    Atm_pour *getPourMachine();

private:
    // State machines
    Atm_led &_valve;
    Atm_digital &_flowmeter;
    Atm_led &_led;
    Atm_mqtt_client &_mqtt;

    Atm_digital flowmeter;
    Atm_timer _flow_update_timer;
    Atm_pour _pour;

    // Callbacks
    static void handleFlowUpdateTimer(int idx, int v, int up);
    static void handleFlow(int idx, int v, int up);
    static void handlePourDone(int idx, int v, int up);
    static void handleFlowStatus(int idx, int v, int up);

    // Need a static pointer to access instance from static callbacks
    static TapService *_instance;
};
