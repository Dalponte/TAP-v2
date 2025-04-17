#pragma once

#include <Automaton.h>

class Atm_tap : public Machine
{
public:
    Atm_tap(void) : Machine() {}

    // States
    enum
    {
        INITIALIZING,
        READY,
        POURING,
        DONE,
        DISCONNECTED,
    };

    // Events
    enum
    {
        EVT_CONNECTED,  // New event: transitions to READY from any state
        EVT_POUR,       // Renamed from EVT_START: transitions from READY to POURING
        EVT_STOP,       // Transitions from POURING to DONE
        EVT_READY,      // New event: transitions from DONE to READY
        EVT_DISCONNECT, // Transitions to DISCONNECTED from any state
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
        CONN_MAX
    };

    Atm_tap &begin();
    int event(int id);
    void action(int id);
    Atm_tap &trace(Stream &stream);

    // Add handler registration for state change
    Atm_tap &onStateChange(atm_cb_push_t callback, int idx = 0);

    // Add connector methods for each state
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

private:
    // Connectors array
    atm_connector connectors[CONN_MAX];
};
