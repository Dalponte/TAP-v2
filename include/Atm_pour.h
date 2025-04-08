#pragma once

#include <Automaton.h>

class Atm_pour : public Machine
{
public:
    Atm_pour(void) : Machine() {};

    // States
    enum
    {
        IDLE,
        POURING,
    };

    // Events
    enum
    {
        EVT_TIMER,
        EVT_START,
        EVT_STOP,
        ELSE
    };

    // Actions
    enum
    {
        ENT_IDLE,
        ENT_POURING,
        EXT_POURING
    };

    // Connectors
    enum
    {
        ON_POUR_DONE,
        ON_FLOW_STATUS,
        CONN_MAX
    };
    atm_connector connectors[CONN_MAX];

    Atm_pour &begin(int initial_timeout_ms = 10000, int continue_timeout_ms = 3000);
    Atm_pour &trace(Stream &stream);
    Atm_pour &flow();
    Atm_pour &start(int pulses, const char *id = ""); // Modified to accept an ID
    Atm_pour &updateFlow();
    const char *getCurrentId(); // New method to get current ID

    // Pour done event connector methods
    Atm_pour &onPourDone(Machine &machine, int event);
    Atm_pour &onPourDone(atm_cb_push_t callback, int idx = 0);

    // Renamed connector methods for flow status updates
    Atm_pour &onFlowStatus(Machine &machine, int event);
    Atm_pour &onFlowStatus(atm_cb_push_t callback, int idx = 0);

    int event(int id);
    void action(int id);

private:
    atm_timer_millis timer; // Timer for pouring timeout
    atm_counter remaining;  // Counter state machine
    int pour_pulses;        // Amount to pour
    int initial_timeout;    // Initial timeout when starting pour
    int continue_timeout;   // Timeout after flow detected
    char current_id[32];    // Storage for the pour ID
};
