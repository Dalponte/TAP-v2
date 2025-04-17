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
        EVT_START,
        EVT_STOP,
        EVT_DISCONNECT,
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

    Atm_tap &begin();
    int event(int id);
    void action(int id);
    Atm_tap &trace(Stream &stream);

    // Add handler registration for state change
    Atm_tap &onStateChange(atm_cb_push_t callback, int idx = 0);
};
