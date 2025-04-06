#pragma once

#include <Automaton.h>

class Atm_flowmeter : public Machine
{

public:
    Atm_flowmeter(void) : Machine() {};

    // States
    enum
    {
        IDLE,
        COUNTING,
        DONE
    };
    // Events
    enum
    {
        EVT_FLOW,
        EVT_RESET,
        EVT_ON,
        EVT_OFF,
        EVT_BUTTON_RELEASE, // New event for button release
        ELSE
    };
    // Actions
    enum
    {
        ENT_INIT,
        ENT_COUNT,
        ENT_FEEDBACK,
        ENT_DONE
    };

    // Connector IDs
    enum
    {
        ON_DONE,
        ON_FLOW, // New connector for flow events
        CONN_MAX
    };

    // Variables
    short pin;
    int flowCount;
    int threshold;
    atm_connector connectors[CONN_MAX];

    // Button variables
    int _last_state;
    atm_timer_millis _debounce_timer;
    const int _debounce = 25; // Default debounce time in ms

    Atm_flowmeter &begin(int attached_pin, int count_threshold = 10);
    Atm_flowmeter &trace(Stream &stream);
    Atm_flowmeter &onDone(Machine &machine, int event = 0);
    Atm_flowmeter &onDone(atm_cb_push_t callback, int idx = 0);
    Atm_flowmeter &onFlow(Machine &machine, int event = 0);
    Atm_flowmeter &onFlow(atm_cb_push_t callback, int idx = 0);

    int event(int id);
    void action(int id);
    int getCount() { return flowCount; }
};
