#pragma once

#include <Automaton.h>
#include "Atm_pour.h"

#define INITIAL_TIMEOUT_MS 10000
#define CONTINUE_TIMEOUT_MS 3000
#define FLOW_UPDATE_INTERVAL_MS 500

#define FLOWMETER_PIN 21 // Flowmeter button simulation pin
#define VALVE_PIN 22     // Mosfet valve simulation pin

class TapService
{
public:
    static TapService &getInstance(
        int initial_timeout_ms = INITIAL_TIMEOUT_MS,
        int continue_timeout_ms = CONTINUE_TIMEOUT_MS,
        int flow_update_interval = FLOW_UPDATE_INTERVAL_MS)
    {
        if (!_instance)
        {
            _instance = new TapService(initial_timeout_ms, continue_timeout_ms, flow_update_interval);
        }
        return *_instance;
    }

    // Start a pour operation
    void startPour(int pulses, const char *id);

    // Get the associated pour machine
    Atm_pour *getPourMachine();

    // Define callback types for publish events
    typedef void (*PourDoneCallback)(const char *id, int pulses, int remaining);
    typedef void (*FlowStatusCallback)(const char *id, int flowRate, int totalPulses);
    typedef void (*PourStartedCallback)(const char *id, int pulses);

    // Set callbacks
    void onPourDone(PourDoneCallback callback);
    void onFlowStatus(FlowStatusCallback callback);
    void onPourStarted(PourStartedCallback callback);

private:
    TapService(
        int initial_timeout_ms,
        int continue_timeout_ms,
        int flow_update_interval);

    TapService(const TapService &) = delete;
    TapService &operator=(const TapService &) = delete;

    Atm_digital flowmeter;
    Atm_timer _flow_update_timer;
    Atm_pour _pour;
    Atm_led _valve_led; // Internal valve LED control

    PourDoneCallback _pourDoneCallback;
    FlowStatusCallback _flowStatusCallback;
    PourStartedCallback _pourStartedCallback;

    static void handleFlowUpdateTimer(int idx, int v, int up);
    static void handleFlow(int idx, int v, int up);
    static void handlePourDone(int idx, int pulses, int remaining);
    static void handleFlowStatus(int idx, int v, int up);
    static void handlePourStarted(int idx, int v, int up);

    static TapService *_instance;
};
