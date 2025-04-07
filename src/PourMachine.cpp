#include "PourMachine.h"
#include <Automaton.h>

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

    // Store timeout values in member variables
    initial_timeout = initial_timeout_ms;
    continue_timeout = continue_timeout_ms;

    return *this;
}

// New method to start a pour with a specific amount
PourMachine &PourMachine::start(int amount)
{
    pour_amount = amount;
    remaining.set(amount);

    Serial.print("Starting pour with amount: ");
    Serial.println(amount);

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
        return remaining.expired(); // Check if counter has reached zero
    }
    return 0;
}

void PourMachine::action(int id)
{
    switch (id)
    {
    case ENT_IDLE:
        Serial.println("PourMachine: Entering IDLE state");
        return;
    case ENT_POURING:
        Serial.println("PourMachine: Entering POURING state");
        timer.set(initial_timeout); // Use the stored initial timeout

        // Make sure the remaining counter is properly set
        if (remaining.value < 1)
        {
            remaining.set(pour_amount);
        }
        return;
    }
}

// Method to update the counter when flow is detected
PourMachine &PourMachine::flow()
{
    if (state() == POURING)
    {
        remaining.decrement();

        Serial.print("Remaining: ");
        Serial.print(remaining.value);
        Serial.print(" / ");
        Serial.println(timer.value);

        // Reset the timer using the stored continue timeout
        timer.setFromNow(this, continue_timeout);

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
