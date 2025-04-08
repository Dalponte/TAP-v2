#include "Atm_pour.h"
#include <Automaton.h>

extern Atm_led valve; // Reference to the valve defined in main.cpp

Atm_pour &Atm_pour::begin(int initial_timeout_ms, int continue_timeout_ms)
{
    // clang-format off
    const static state_t state_table[] PROGMEM = {
    /*           ON_ENTER    ON_LOOP  ON_EXIT     EVT_TIMER  EVT_START  EVT_STOP  ELSE */
    /* IDLE    */ ENT_IDLE,      -1,      -1,           -1,   POURING,      -1,    -1,
    /* POURING */ ENT_POURING,   -1, EXT_POURING,     IDLE,        -1,     IDLE,   -1,
    };
    // clang-format on

    Machine::begin(state_table, ELSE);

    initial_timeout = initial_timeout_ms;
    continue_timeout = continue_timeout_ms;

    return *this;
}

// Add onPourDone connector implementations
Atm_pour &Atm_pour::onPourDone(Machine &machine, int event)
{
    onPush(connectors, ON_POUR_DONE, 0, 0, 0, machine, event);
    return *this;
}

Atm_pour &Atm_pour::onPourDone(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_POUR_DONE, 0, 0, 0, callback, idx);
    return *this;
}

// Add connector methods for remaining change events
Atm_pour &Atm_pour::onRemainingChange(Machine &machine, int event)
{
    onPush(connectors, ON_REMAINING_CHANGE, 0, 0, 0, machine, event);
    return *this;
}

Atm_pour &Atm_pour::onRemainingChange(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_REMAINING_CHANGE, 0, 0, 0, callback, idx);
    return *this;
}

Atm_pour &Atm_pour::start(int pulses)
{
    pour_pulses = pulses;
    remaining.set(pulses);

    Serial.print("Starting pour with pulses amount: ");
    Serial.println(pulses);

    trigger(EVT_START);
    return *this;
}

int Atm_pour::event(int id)
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

void Atm_pour::action(int id)
{
    switch (id)
    {
    case ENT_IDLE:
        Serial.println("Atm_pour: Entering IDLE state");
        valve.trigger(valve.EVT_OFF); // Close valve
        return;
    case ENT_POURING:
        Serial.println("Atm_pour: Entering POURING state");
        valve.trigger(valve.EVT_ON); // Open valve
        timer.set(initial_timeout);

        // Make sure the remaining counter is properly set
        if (remaining.value < 1)
        {
            remaining.set(pour_pulses);
        }
        return;
    case EXT_POURING:
        // Push the POUR_DONE event when exiting the POURING state
        // Send: pulses poured as v, remaining pulses as up
        push(connectors, ON_POUR_DONE, 0, pour_pulses - remaining.value, remaining.value);
        Serial.println("Atm_pour: Exiting POURING state");
        return;
    }
}

Atm_pour &Atm_pour::flow()
{
    if (state() == POURING)
    {
        remaining.decrement();
        timer.setFromNow(this, continue_timeout);

        Serial.print("Remaining: ");
        Serial.println(remaining.value);

        // Push the remaining value through the ON_REMAINING_CHANGE connector
        push(connectors, ON_REMAINING_CHANGE, 0, remaining.value, pour_pulses - remaining.value);

        // Check if remaining has expired after this decrement
        if (remaining.expired())
        {
            trigger(EVT_STOP);
        }
    }
    return *this;
}

Atm_pour &Atm_pour::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "POUR\0EVT_TIMER\0EVT_START\0EVT_STOP\0EVT_FLOW_UPDATE\0ELSE\0IDLE\0POURING");
    return *this;
}
