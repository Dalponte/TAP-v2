#include "Atm_tap.h"
#include <Automaton.h>

Atm_tap &Atm_tap::begin()
{
    // clang-format off
    const static state_t state_table[] PROGMEM = {
        /*                   ON_ENTER            ON_LOOP  ON_EXIT  EVT_CONNECTED  EVT_POUR  EVT_STOP  EVT_READY  EVT_DISCONNECT  ELSE */
        /* INITIALIZING   */ ENT_INITIALIZING,      -1,      -1,      READY,        -1,       -1,       -1,        DISCONNECTED,   -1,
        /* READY          */ ENT_READY,             -1,      -1,      -1,           POURING,  -1,       -1,        DISCONNECTED,   -1,
        /* POURING        */ ENT_POURING,           -1,      -1,      -1,           -1,       DONE,     -1,        -1,             -1,
        /* DONE           */ ENT_DONE,              -1,      -1,      -1,           -1,       -1,       READY,     -1,             -1,
        /* DISCONNECTED   */ ENT_DISCONNECTED,      -1,      -1,      READY,        -1,       -1,       -1,        -1,             -1,
    };
    // clang-format on
    Machine::begin(state_table, ELSE);
    state(INITIALIZING);

    return *this;
}

int Atm_tap::event(int id)
{
    return 0;
}

void Atm_tap::action(int id)
{
    switch (id)
    {
    case ENT_INITIALIZING:
        push(connectors, ON_INITIALIZING, 0, 0, 0);
        return;
    case ENT_READY:
        push(connectors, ON_READY, 0, 0, 0);
        return;
    case ENT_POURING:
        push(connectors, ON_POURING, 0, 0, 0);
        return;
    case ENT_DONE:
        push(connectors, ON_DONE, 0, 0, 0);
        return;
    case ENT_DISCONNECTED:
        push(connectors, ON_DISCONNECTED, 0, 0, 0);
        return;
    }
    // This will push the current state and action ID for general state change handling
    push(connectors, ON_STATE_CHANGE, 0, state(), id);
}

Atm_tap &Atm_tap::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "TAP\0EVT_CONNECTED\0EVT_POUR\0EVT_STOP\0EVT_READY\0EVT_DISCONNECT\0ELSE\0INITIALIZING\0READY\0POURING\0DONE\0DISCONNECTED");
    return *this;
}

// Implement onStateChange method
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
