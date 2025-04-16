#include "TapService.h"

// Initialize static member
TapService *TapService::_instance = nullptr;

TapService::TapService()
    : _pourDoneCallback(nullptr), _flowStatusCallback(nullptr), _pourStartedCallback(nullptr)
{
}

void TapService::begin(
    int initial_timeout_ms,
    int continue_timeout_ms,
    int flow_update_interval)
{
    // Initialize valve LED
    _valve_led.begin(VALVE_PIN);
    _valve_led.trigger(_valve_led.EVT_OFF);

    // Setup flow meter
    flowmeter.begin(FLOWMETER_PIN, 1, false, true)
        .onChange(HIGH, handleFlow);

    // Setup flow update timer
    _flow_update_timer.begin(flow_update_interval)
        .repeat(ATM_COUNTER_OFF)
        .onTimer(handleFlowUpdateTimer);

    // Setup pour process
    _pour.begin(initial_timeout_ms, continue_timeout_ms)
        .onPourDone(handlePourDone)
        .onFlowStatus(handleFlowStatus)
        .onPourStart(handlePourStarted)
        .trace(Serial);
}

void TapService::startPour(int pulses, const char *id)
{
    _valve_led.trigger(_valve_led.EVT_ON); // Open valve
    _pour.start(pulses, id);
    _flow_update_timer.start();
}

Atm_pour *TapService::getPourMachine()
{
    return &_pour;
}

void TapService::onPourDone(PourDoneCallback callback)
{
    _pourDoneCallback = callback;
}

void TapService::onFlowStatus(FlowStatusCallback callback)
{
    _flowStatusCallback = callback;
}

void TapService::onPourStarted(PourStartedCallback callback)
{
    _pourStartedCallback = callback;
}

// Static callback handlers
void TapService::handleFlow(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_pour.flow();
    }
}

void TapService::handleFlowUpdateTimer(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_pour.updateFlow();
    }
}

void TapService::handlePourDone(int idx, int pulses, int remaining)
{
    if (_instance)
    {
        _instance->_flow_update_timer.stop();
        _instance->_valve_led.trigger(_instance->_valve_led.EVT_OFF); // Close valve

        if (_instance->_pourDoneCallback)
        {
            _instance->_pourDoneCallback(
                _instance->_pour.getCurrentId(),
                pulses,
                remaining);
        }

        Serial.println();
        Serial.print("Pour completed! Pulses poured: ");
        Serial.print(pulses);
        Serial.print(", Remaining: ");
        Serial.print(remaining);
        Serial.print(", ID: ");
        Serial.println(_instance->_pour.getCurrentId());
    }
}

void TapService::handleFlowStatus(int idx, int v, int up)
{
    if (_instance)
    {
        // Call the callback if it's registered
        if (_instance->_flowStatusCallback)
        {
            _instance->_flowStatusCallback(
                _instance->_pour.getCurrentId(),
                up,
                v);
        }
    }
}

void TapService::handlePourStarted(int idx, int v, int up)
{
    if (_instance && _instance->_pourStartedCallback)
    {
        _instance->_pourStartedCallback(_instance->_pour.getCurrentId(), v);
    }
}
