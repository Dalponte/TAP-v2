#include <Automaton.h>
#include "Atm_flowmeter.h"

Atm_flowmeter &Atm_flowmeter::begin(int attached_pin, int count_threshold)
{
    // clang-format off
  const static state_t state_table[] PROGMEM = {
    /*             ON_ENTER    ON_LOOP    ON_EXIT    EVT_FLOW    EVT_RESET   EVT_ON      EVT_OFF  EVT_BUTTON_RELEASE  ELSE */
    /* IDLE    */  ENT_INIT,       -1,        -1,         -1,         -1,    COUNTING,      -1,           -1,         -1,
    /* COUNTING*/       -1,        -1,        -1,    ENT_COUNT,      IDLE,      -1,        IDLE,       ENT_COUNT,      -1,
    /* DONE    */  ENT_DONE,       -1,        -1,         -1,        IDLE,      -1,         -1,           -1,         -1,
  };
    // clang-format on

    Machine::begin(state_table, ELSE);
    pin = attached_pin;
    flowCount = 0;
    threshold = count_threshold;
    pinMode(pin, INPUT_PULLUP);

    // Initialize button state
    _last_state = digitalRead(pin);

    return *this;
}

int Atm_flowmeter::event(int id)
{
    switch (id)
    {
    case EVT_FLOW:
        return 0; // This is triggered externally
    case EVT_RESET:
        return 0; // This is triggered externally
    case EVT_BUTTON_RELEASE:
        // Check if button was released (HIGH is released with INPUT_PULLUP)
        int buttonState = digitalRead(pin);
        Serial.print("Button state: ");
        Serial.println(buttonState);
        if (_debounce_timer.expired(this) && buttonState == HIGH && _last_state == LOW)
        {
            _last_state = buttonState;
            _debounce_timer.set(_debounce);
            return 1; // Button was released
        }
        _last_state = buttonState;
        return 0;
    }
    return 0;
}

void Atm_flowmeter::action(int id)
{
    switch (id)
    {
    case ENT_INIT:
        flowCount = 0;
        return;
    case ENT_COUNT:
        flowCount++;
        Serial.print("Flow count: ");
        Serial.println(flowCount);

        // Call onFlow callback to notify about the flow event
        push(connectors, ON_FLOW, 0, flowCount, 0);

        // If threshold reached, go to DONE state
        if (flowCount >= threshold)
        {
            state(DONE);
        }
        return;
    case ENT_DONE:
        // Call the callback with the correct connector ID (ON_DONE)
        push(connectors, ON_DONE, 0, flowCount, 0);
        Serial.println("**** FLOW THRESHOLD REACHED! ****");
        return;
    }
}

Atm_flowmeter &Atm_flowmeter::onDone(Machine &machine, int event)
{
    // Correct onPush signature with all required parameters
    onPush(connectors, ON_DONE, 0, 1, 1, machine, event);
    return *this;
}

Atm_flowmeter &Atm_flowmeter::onDone(atm_cb_push_t callback, int idx)
{
    // Correct onPush signature with all required parameters
    onPush(connectors, ON_DONE, 0, 1, 1, callback, idx);
    return *this;
}

Atm_flowmeter &Atm_flowmeter::onFlow(Machine &machine, int event)
{
    // Add connector for flow events
    onPush(connectors, ON_FLOW, 0, 1, 1, machine, event);
    return *this;
}

Atm_flowmeter &Atm_flowmeter::onFlow(atm_cb_push_t callback, int idx)
{
    // Add connector for flow events
    onPush(connectors, ON_FLOW, 0, 1, 1, callback, idx);
    return *this;
}

Atm_flowmeter &Atm_flowmeter::trace(Stream &stream)
{
    Machine::setTrace(&stream, atm_serial_debug::trace,
                      "FLOWMETER\0EVT_FLOW\0EVT_RESET\0EVT_ON\0EVT_OFF\0EVT_BUTTON_RELEASE\0ELSE\0IDLE\0COUNTING\0DONE");
    return *this;
}
