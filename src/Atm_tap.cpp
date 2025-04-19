#include "Atm_tap.h"
#include <Automaton.h>

// Update begin signature to accept LedService reference
Atm_tap &Atm_tap::begin(LedService &led, int initial_timeout_ms, int continue_timeout_ms)
{
    // Store the LedService reference
    ledService = &led;

    // clang-format off
    const static state_t state_table[] PROGMEM = {
        /*                   ON_ENTER            ON_LOOP  ON_EXIT  EVT_CONNECTED  EVT_POUR  EVT_STOP  EVT_READY  EVT_DISCONNECT  EVT_TIMER  ELSE */
        /* INITIALIZING   */ ENT_INITIALIZING,      -1,      -1,      READY,        -1,       -1,       -1,        DISCONNECTED,   -1,        -1,
        /* READY          */ ENT_READY,             -1,      -1,      -1,           POURING,  -1,       -1,        DISCONNECTED,   -1,        -1, // EVT_POUR transitions to POURING
        /* POURING        */ ENT_POURING,           -1,      -1,      -1,           -1,       DONE,     -1,        -1,             DONE,      -1, // EVT_STOP or EVT_TIMER transitions to DONE
        /* DONE           */ ENT_DONE,              -1,      -1,      -1,           -1,       -1,       READY,     -1,             -1,        -1, // EVT_READY transitions to READY
        /* DISCONNECTED   */ ENT_DISCONNECTED,      -1,      -1,      READY,        -1,       -1,       -1,        -1,             -1,        -1,
    };
    // clang-format on
    Machine::begin(state_table, ELSE);
    state(INITIALIZING);

    initial_timeout = initial_timeout_ms;
    continue_timeout = continue_timeout_ms;

    current_id = 0;

    // Initialize timer and counter (optional, but good practice)
    timer.set(0);
    remaining.set(0);

    return *this;
}

int Atm_tap::event(int id)
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

void Atm_tap::action(int id)
{
    switch (id)
    {
    case ENT_INITIALIZING:
        ledService->red();
        return;

    case ENT_READY:
        ledService->blue();
        return;

    case ENT_POURING:
        timer.set(initial_timeout);
        if (remaining.value < 1)
        {
            remaining.set(pour_pulses); // Ensure counter is set if start wasn't called right before
        }
        ledService->green();
        return;

    case ENT_DONE:
        Serial.println("Atm_tap: Entering DONE state");
        ledService->red();
        // Send: pulses poured as v, remaining pulses as up using ON_DONE
        push(connectors, ON_DONE, 0, pour_pulses - remaining.value, remaining.value);
        return;

    case ENT_DISCONNECTED:
        Serial.println("Atm_tap: Entering DISCONNECTED state");
        ledService->setBlue(true);
        ledService->setRed(true);
        ledService->setGreen(true);
        return;
    }

    push(connectors, ON_STATE_CHANGE, 0, id, state());
}

Atm_tap &Atm_tap::start(int pulses, int id)
{
    if (state() == READY) // Only start if ready
    {
        pour_pulses = pulses;
        remaining.set(pulses);

        current_id = id;

        Serial.print("Atm_tap: Starting pour > pulses: ");
        Serial.print(pulses);
        Serial.print(", ID: ");
        Serial.println(id);

        trigger(EVT_POUR);
    }
    else
    {
        Serial.print("Atm_tap: Cannot start pour, not in READY state. Current state: ");
        Serial.println(state());
    }
    return *this;
}

Atm_tap &Atm_tap::flow()
{
    if (state() == POURING)
    {
        remaining.decrement();
        timer.setFromNow(this, continue_timeout);
        Serial.print(" >");

        // Push flow status update
        updateFlow();

        // Check if remaining has expired after this decrement
        if (remaining.expired())
        {
            Serial.println("\nAtm_tap: Pour finished (flow count reached)");
            trigger(EVT_STOP);
        }
    }
    return *this;
}

Atm_tap &Atm_tap::updateFlow()
{
    // Push: remaining pulses as v, poured pulses as up
    push(connectors, ON_FLOW_STATUS, 0, remaining.value, pour_pulses - remaining.value);
    return *this;
}

int Atm_tap::getCurrentId()
{
    return current_id;
}

Atm_tap &Atm_tap::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      // Add EVT_TIMER to the trace string
                      "TAP\0EVT_CONNECTED\0EVT_POUR\0EVT_STOP\0EVT_READY\0EVT_DISCONNECT\0EVT_TIMER\0ELSE\0INITIALIZING\0READY\0POURING\0DONE\0DISCONNECTED");
    return *this;
}

Atm_tap &Atm_tap::onStateChange(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_STATE_CHANGE, 0, 0, 0, callback, idx);
    return *this;
}

// Initializing state connectors
Atm_tap &Atm_tap::onInitializing(Machine &machine, int event)
{
    onPush(connectors, ON_INITIALIZING, 0, 0, 0, machine, event);
    return *this;
}
Atm_tap &Atm_tap::onInitializing(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_INITIALIZING, 0, 0, 0, callback, idx);
    return *this;
}

// Ready state connectors
Atm_tap &Atm_tap::onReady(Machine &machine, int event)
{
    onPush(connectors, ON_READY, 0, 0, 0, machine, event);
    return *this;
}
Atm_tap &Atm_tap::onReady(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_READY, 0, 0, 0, callback, idx);
    return *this;
}

// Pouring state connectors
Atm_tap &Atm_tap::onPouring(Machine &machine, int event)
{
    onPush(connectors, ON_POURING, 0, 0, 0, machine, event);
    return *this;
}
Atm_tap &Atm_tap::onPouring(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_POURING, 0, 0, 0, callback, idx);
    return *this;
}

// Done state connectors
Atm_tap &Atm_tap::onDone(Machine &machine, int event)
{
    onPush(connectors, ON_DONE, 0, 0, 0, machine, event);
    return *this;
}
Atm_tap &Atm_tap::onDone(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_DONE, 0, 0, 0, callback, idx);
    return *this;
}

// Disconnected state connectors
Atm_tap &Atm_tap::onDisconnected(Machine &machine, int event)
{
    onPush(connectors, ON_DISCONNECTED, 0, 0, 0, machine, event);
    return *this;
}
Atm_tap &Atm_tap::onDisconnected(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_DISCONNECTED, 0, 0, 0, callback, idx);
    return *this;
}

// Implement new connector methods from Atm_pour
Atm_tap &Atm_tap::onFlowStatus(Machine &machine, int event)
{
    onPush(connectors, ON_FLOW_STATUS, 0, 0, 0, machine, event);
    return *this;
}

Atm_tap &Atm_tap::onFlowStatus(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_FLOW_STATUS, 0, 0, 0, callback, idx);
    return *this;
}