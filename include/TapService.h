#pragma once

#include <Automaton.h>
#include "Atm_pour.h"

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

    void init(Atm_led &valve, Atm_led &led);

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
    // Private constructor for singleton
    TapService();

    // No copy or assignment
    TapService(const TapService &) = delete;
    TapService &operator=(const TapService &) = delete;

    // References to external objects
    Atm_led *_valve = nullptr;
    Atm_led *_led = nullptr;

    // State machines
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

    static TapService *_instance;
};
