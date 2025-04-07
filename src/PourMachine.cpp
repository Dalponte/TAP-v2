#include "PourMachine.h"
#include <Automaton.h>

extern Atm_led valve; // Reference to the valve defined in main.cpp

PourMachine &PourMachine::begin(int initial_timeout_ms, int continue_timeout_ms)
{
    // clang-format off
    const static state_t state_table[] PROGMEM = {
    /*           ON_ENTER    ON_LOOP  ON_EXIT  EVT_TIMER  EVT_START  EVT_STOP  ELSE */
    /* IDLE    */ ENT_IDLE,      -1,      -1,        -1,   POURING,      -1,    -1,
    /* POURING */ ENT_POURING,   -1,      -1,      IDLE,        -1,     IDLE,   -1,
    };
    // clang-format on

    Machine::begin(state_table, ELSE);

    initial_timeout = initial_timeout_ms;
    continue_timeout = continue_timeout_ms;

    return *this;
}

PourMachine &PourMachine::start(int pulses)
{
    pour_pulses = pulses;
    remaining.set(pulses);

    Serial.print("Starting pour with pulses amount: ");
    Serial.println(pulses);

    trigger(EVT_START);
    return *this;
}

int PourMachine::event(int id)
{
    switch (id)
    {
    case EVT_TIMER:
        return timer.expired(this);
    case EVT_STOP:
        return remaining.expired();
    }
    return 0;
}

void PourMachine::action(int id)
{
    switch (id)
    {
    case ENT_IDLE:
        Serial.println("PourMachine: Entering IDLE state");
        valve.trigger(valve.EVT_OFF); // Turn valve OFF when idle
        return;
    case ENT_POURING:
        Serial.println("PourMachine: Entering POURING state");
        valve.trigger(valve.EVT_ON); // Turn valve ON when pouring
        timer.set(initial_timeout);

        // Make sure the remaining counter is properly set
        if (remaining.value < 1)
        {
            remaining.set(pour_pulses);
        }
        return;
    }
}

PourMachine &PourMachine::flow()
{
    if (state() == POURING)
    {
        remaining.decrement();
        timer.setFromNow(this, continue_timeout);

        Serial.print("Remaining: ");
        Serial.println(remaining.value);

        // Check if remaining has expired after this decrement
        if (remaining.expired())
        {
            trigger(EVT_STOP);
        }
    }
    return *this;
}

PourMachine &PourMachine::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "POUR\0EVT_TIMER\0EVT_START\0EVT_STOP\0ELSE\0IDLE\0POURING");
    return *this;
}
