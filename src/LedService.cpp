#include "LedService.h"

// Initialize static instance pointer
LedService *LedService::instance = nullptr;

LedService::LedService()
{
    // Initialize all LEDs
    led.begin(PIN_LED);
    led_blue.begin(PIN_BLUE);
    led_green.begin(PIN_GREEN);
    led_red.begin(PIN_RED);
}

LedService &LedService::getInstance()
{
    if (instance == nullptr)
    {
        instance = new LedService();
    }
    return *instance;
}

// Remove initialize() method

void LedService::setGreen(bool state)
{
    led_green.trigger(state ? led_green.EVT_ON : led_green.EVT_OFF);
}

void LedService::setBlue(bool state)
{
    led_blue.trigger(state ? led_blue.EVT_ON : led_blue.EVT_OFF);
}

void LedService::setRed(bool state)
{
    led_red.trigger(state ? led_red.EVT_ON : led_red.EVT_OFF);
}

void LedService::green()
{
    setRed(false);
    setBlue(false);
    setGreen(true);
}

void LedService::blue()
{
    setRed(false);
    setGreen(false);
    setBlue(true);
}

void LedService::red()
{
    setGreen(false);
    setBlue(false);
    setRed(true);
}
