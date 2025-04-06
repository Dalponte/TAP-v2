#include <Arduino.h>
#include <Automaton.h>
#include "setup.h"

void initialize(
    Atm_button &flow,
    Atm_button &rfid,
    Atm_led &valve,
    Atm_led &led,
    Atm_led &led_blue,
    Atm_led &led_green,
    Atm_led &led_red)
{
    valve.begin(VALVE_PIN);
    led.begin(PIN_LED);
    led_blue.begin(PIN_BLUE);
    led_green.begin(PIN_GREEN);
    led_red.begin(PIN_RED);

    valve.trigger(valve.EVT_ON);
    led.trigger(led.EVT_ON);
    led_blue.trigger(led.EVT_ON);
    led_green.trigger(led.EVT_ON);
    led_red.trigger(led.EVT_ON);

    flow.begin(FLOWMETER_PIN);
    rfid.begin(RFID_BUTTON);
}
