#pragma once

#include <Automaton.h>
#include "Atm_pour.h"

class TapService
{
public:
    // Remove mqtt parameter from constructor
    TapService(Atm_led &valve, Atm_led &led);

    // Initialize the service
    void begin(
        int initial_timeout_ms,
        int continue_timeout_ms,
        int flow_update_interval);

    // Start a pour operation
    void startPour(int pulses, const char *id);

    // Get the associated pour machine
    Atm_pour *getPourMachine();

    // Define callback types for publish events
    typedef void (*PourDoneCallback)(const char *id, int pulses, int remaining);
    typedef void (*FlowStatusCallback)(const char *id, int flowRate, int totalPulses);

    // Set callbacks
    void onPourDone(PourDoneCallback callback);
    void onFlowStatus(FlowStatusCallback callback);

private:
    // State machines
    Atm_led &_valve;
    Atm_digital &_flowmeter;
    Atm_led &_led;

    Atm_digital flowmeter;
    Atm_timer _flow_update_timer;
    Atm_pour _pour;

    // Callbacks for publishing events
    PourDoneCallback _pourDoneCallback;
    FlowStatusCallback _flowStatusCallback;

    // Static callback handlers
    static void handleFlowUpdateTimer(int idx, int v, int up);
    static void handleFlow(int idx, int v, int up);
    static void handlePourDone(int idx, int pulses, int remaining);
    static void handleFlowStatus(int idx, int v, int up);

    // Need a static pointer to access instance from static callbacks
    static TapService *_instance;
};
