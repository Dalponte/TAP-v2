#include "TapMachine.h"

TapMachine &TapMachine::begin()
{
    // clang-format off
    const static state_t state_table[] PROGMEM = {
        /*              ON_ENTER      ON_LOOP  ON_EXIT  EVT_NEXT  ELSE */
        /* INIT     */  ENT_INIT,         -1,      -1,     IDLE,    -1,
        /* IDLE     */  ENT_IDLE,         -1,      -1,  POURING,    -1,
        /* POURING  */  ENT_POURING,      -1,      -1,     DONE,    -1,
        /* DONE     */  ENT_DONE,         -1,      -1,     IDLE,    -1,
    };
    // clang-format on

    // Initialize LEDs
    led.begin(PIN_LED).blink(40, 250);
    led_blue.begin(PIN_RGB_BLUE);
    led_green.begin(PIN_RGB_GREEN);
    led_red.begin(PIN_RGB_RED);

    Machine::begin(state_table, ELSE);
    timer.set(3000); // 3 seconds
    return *this;
}

int TapMachine::event(int id)
{
    switch (id)
    {
    case EVT_NEXT:
        return timer.expired(this);
    }
    return 0;
}

void TapMachine::action(int id)
{
    switch (id)
    {
    case ENT_INIT:
        Serial.println("State: INIT");
        led.trigger(led.EVT_BLINK);
        led_blue.trigger(led_blue.EVT_ON);
        led_green.trigger(led_green.EVT_ON);
        led_red.trigger(led_red.EVT_OFF);
        timer.set(3000);
        return;
    case ENT_IDLE:
        Serial.println("State: IDLE");
        led_blue.trigger(led_blue.EVT_OFF);
        led_green.trigger(led_green.EVT_OFF);
        led_red.trigger(led_red.EVT_ON);
        timer.set(3000);
        return;
    case ENT_POURING:
        Serial.println("State: POURING");
        led_green.trigger(led_green.EVT_TOGGLE);

        timer.set(3000);
        return;
    case ENT_DONE:
        Serial.println("State: DONE");
        led_green.trigger(led_green.EVT_TOGGLE);
        led_blue.trigger(led_blue.EVT_TOGGLE);

        timer.set(3000);
        return;
    }
}
