#include "Atm_pour.h"
#include <Automaton.h>

extern Atm_led valve; // Temporary global variable for valve control

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

    // Initialize ID to empty string
    current_id[0] = '\0';

    return *this;
}

const char *Atm_pour::getCurrentId()
{
    return current_id;
}

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

Atm_pour &Atm_pour::onFlowStatus(Machine &machine, int event)
{
    onPush(connectors, ON_FLOW_STATUS, 0, 0, 0, machine, event);
    return *this;
}

Atm_pour &Atm_pour::onFlowStatus(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_FLOW_STATUS, 0, 0, 0, callback, idx);
    return *this;
}

Atm_pour &Atm_pour::start(int pulses, const char *id)
{
    pour_pulses = pulses;
    remaining.set(pulses);

    // Store the ID
    strncpy(current_id, id, sizeof(current_id) - 1);
    current_id[sizeof(current_id) - 1] = '\0'; // Ensure null-termination

    Serial.print("Atm_pour: Starting > pulses: ");
    Serial.print(pulses);
    Serial.print(", ID: ");
    Serial.println(id);

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
        Serial.println("Atm_pour: Valve closed");
        valve.trigger(valve.EVT_OFF); // Close valve
        return;
    case ENT_POURING:
        Serial.println("Atm_pour: Valve opened");
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

Atm_pour &Atm_pour::updateFlow()
{
    push(connectors, ON_FLOW_STATUS, 0, remaining.value, pour_pulses - remaining.value);
    return *this;
}

Atm_pour &Atm_pour::flow()
{
    if (state() == POURING)
    {
        remaining.decrement();
        timer.setFromNow(this, continue_timeout);
        Serial.print(" >");

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
