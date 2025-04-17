#include "Atm_tap.h"
#include <Automaton.h>

Atm_tap &Atm_tap::begin()
{
    // clang-format off
    const static state_t state_table[] PROGMEM = {
        /*                   ON_ENTER            ON_LOOP  ON_EXIT  EVT_START  EVT_STOP  EVT_DISCONNECT  ELSE */
        /* INITIALIZING   */ ENT_INITIALIZING,      -1,      -1,    READY,      -1,         DISCONNECTED, -1,
        /* READY          */ ENT_READY,             -1,      -1,    POURING,    -1,         DISCONNECTED, -1,
        /* POURING        */ ENT_POURING,           -1,      -1,    -1,         DONE,       DISCONNECTED, -1,
        /* DONE           */ ENT_DONE,              -1,      -1,    READY,      -1,         DISCONNECTED, -1,
        /* DISCONNECTED   */ ENT_DISCONNECTED,      -1,      -1,    INITIALIZING, -1,       -1,           -1,
    };
    // clang-format on
    Machine::begin(state_table, ELSE);
    state(INITIALIZING);

    return *this;
}

enum
{
    ON_STATE_CHANGE,
    CONN_MAX
};
atm_connector connectors[CONN_MAX];

int Atm_tap::event(int id)
{
    return 0;
}

void Atm_tap::action(int id)
{
    switch (id)
    {
    case ENT_INITIALIZING:
        return;
    case ENT_READY:
        return;
    case ENT_POURING:
        return;
    case ENT_DONE:
        return;
    case ENT_DISCONNECTED:
        return;
    }
    push(connectors, ON_STATE_CHANGE, 0, id, 0);
}

Atm_tap &Atm_tap::onStateChange(atm_cb_push_t callback, int idx)
{
    onPush(connectors, ON_STATE_CHANGE, 0, 0, 0, callback, idx);
    return *this;
}

Atm_tap &Atm_tap::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "TAP\0EVT_START\0EVT_STOP\0EVT_DISCONNECT\0ELSE\0INITIALIZING\0READY\0POURING\0DONE\0DISCONNECTED");
    return *this;
}
