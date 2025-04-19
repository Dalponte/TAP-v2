#pragma once

#include <Automaton.h>
#include "LedService.h" // Add include for LedService

class Atm_tap : public Machine
{
public:
    Atm_tap(void) : Machine() {}

    // States
    enum
    {
        INITIALIZING,
        READY,   // Maps to Atm_pour::IDLE
        POURING, // Maps to Atm_pour::POURING
        DONE,
        DISCONNECTED,
    };

    // Events
    enum
    {
        EVT_CONNECTED,
        EVT_POUR,  // Triggered by start()
        EVT_STOP,  // Triggered by flow() when remaining expires or external stop
        EVT_READY, // Triggered after DONE state
        EVT_DISCONNECT,
        EVT_TIMER, // When pouring ends by timeout
        ELSE
    };

    // Actions
    enum
    {
        ENT_INITIALIZING,
        ENT_READY,
        ENT_POURING,
        ENT_DONE,
        ENT_DISCONNECTED
    };

    // Connector IDs
    enum
    {
        ON_STATE_CHANGE,
        ON_INITIALIZING,
        ON_READY,
        ON_POURING,
        ON_DONE,
        ON_DISCONNECTED,
        ON_FLOW_STATUS,
        CONN_MAX
    };

    // Modify begin signature to accept LedService and timeouts
    Atm_tap &begin(LedService &led, int initial_timeout_ms = 10000, int continue_timeout_ms = 3000);
    int event(int id);
    void action(int id);
    Atm_tap &trace(Stream &stream);

    // Add methods from Atm_pour
    Atm_tap &start(int pulses, int id); // Changed id type to int
    Atm_tap &flow();
    Atm_tap &updateFlow();
    int getCurrentId(); // Changed return type to int

    // Existing handler registration
    Atm_tap &onStateChange(atm_cb_push_t callback, int idx = 0);
    Atm_tap &onInitializing(Machine &machine, int event = 0);
    Atm_tap &onInitializing(atm_cb_push_t callback, int idx = 0);
    Atm_tap &onReady(Machine &machine, int event = 0);
    Atm_tap &onReady(atm_cb_push_t callback, int idx = 0);
    Atm_tap &onPouring(Machine &machine, int event = 0);
    Atm_tap &onPouring(atm_cb_push_t callback, int idx = 0);
    Atm_tap &onDone(Machine &machine, int event = 0);
    Atm_tap &onDone(atm_cb_push_t callback, int idx = 0);
    Atm_tap &onDisconnected(Machine &machine, int event = 0);
    Atm_tap &onDisconnected(atm_cb_push_t callback, int idx = 0);

    Atm_tap &onFlowStatus(Machine &machine, int event);
    Atm_tap &onFlowStatus(atm_cb_push_t callback, int idx = 0);

private:
    // Connectors array - size updated by CONN_MAX
    atm_connector connectors[CONN_MAX];

    // Add members from Atm_pour
    atm_timer_millis timer; // Timer for pouring timeout
    atm_counter remaining;  // Counter state machine
    int pour_pulses;        // Amount to pour
    int initial_timeout;    // Initial timeout when starting pour
    int continue_timeout;   // Timeout after flow detected
    int current_id;         // Storage for the pour ID as integer
    LedService *ledService; // Pointer to the LedService instance
};
