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
        ENT_POURING
    };

    Atm_pour &begin(int initial_timeout_ms = 10000, int continue_timeout_ms = 3000);
    Atm_pour &trace(Stream &stream);
    Atm_pour &flow();
    Atm_pour &start(int pulses);

    int event(int id);
    void action(int id);

private:
    atm_timer_millis timer; // Timer for pouring timeout
    atm_counter remaining;  // Counter state machine
    int pour_pulses;        // Amount to pour
    int initial_timeout;    // Initial timeout when starting pour
    int continue_timeout;   // Timeout after flow detected
};
