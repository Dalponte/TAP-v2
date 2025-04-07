#pragma once

#include <Automaton.h>

class PourMachine : public Machine
{
public:
    PourMachine(void) : Machine() {};

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

    PourMachine &begin(int initial_timeout_ms = 10000, int continue_timeout_ms = 3000);
    PourMachine &trace(Stream &stream);
    PourMachine &flow();            // Method to handle flow events
    PourMachine &start(int amount); // Method to start pour with amount

    int event(int id);
    void action(int id);

private:
    atm_timer_millis timer; // Timer for pouring timeout
    atm_counter remaining;  // Counter state machine
    int pour_amount;        // Amount to pour
    int initial_timeout;    // Initial timeout when starting pour
    int continue_timeout;   // Timeout after flow detected
};
