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
    static TapService &getInstance()
    {
        if (!_instance)
        {
            _instance = new TapService();
        }
        return *_instance;
    }

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
    typedef void (*PourStartedCallback)(const char *id, int pulses);

    // Set callbacks
    void onPourDone(PourDoneCallback callback);
    void onFlowStatus(FlowStatusCallback callback);
    void onPourStarted(PourStartedCallback callback);

private:
    // Private constructor for singleton
    TapService();

    // No copy or assignment
    TapService(const TapService &) = delete;
    TapService &operator=(const TapService &) = delete;

    // State machines
    Atm_digital flowmeter;
    Atm_timer _flow_update_timer;
    Atm_pour _pour;
    Atm_led _valve_led; // Internal valve LED control

    // Callbacks for publishing events
    PourDoneCallback _pourDoneCallback;
    FlowStatusCallback _flowStatusCallback;
    PourStartedCallback _pourStartedCallback;

    // Static callback handlers
    static void handleFlowUpdateTimer(int idx, int v, int up);
    static void handleFlow(int idx, int v, int up);
    static void handlePourDone(int idx, int pulses, int remaining);
    static void handleFlowStatus(int idx, int v, int up);
    static void handlePourStarted(int idx, int v, int up);

    static TapService *_instance;
};
