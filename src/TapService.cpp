#include "TapService.h"
#include "setup.h"

// Initialize static member
TapService *TapService::_instance = nullptr;

TapService::TapService()
    : _pourDoneCallback(nullptr), _flowStatusCallback(nullptr)
{
}

void TapService::init(Atm_led &valve, Atm_led &led)
{
    _valve = &valve;
    _led = &led;
}

void TapService::begin(
    int initial_timeout_ms,
    int continue_timeout_ms,
    int flow_update_interval)
{
    // Setup flow meter
    flowmeter.begin(FLOWMETER_PIN, 1, false, true)
        .onChange(HIGH, handleFlow);

    // Setup flow update timer
    _flow_update_timer.begin(flow_update_interval)
        .repeat(ATM_COUNTER_OFF)
        .onTimer(handleFlowUpdateTimer);

    // Setup pour process
    _pour.begin(initial_timeout_ms, continue_timeout_ms)
        // .trace(Serial)
        .onPourDone(handlePourDone)
        .onFlowStatus(handleFlowStatus);
}

void TapService::startPour(int pulses, const char *id)
{
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

// Static callback handlers
void TapService::handleFlow(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_led->trigger(_instance->_led->EVT_TOGGLE);
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

        // Call the callback if it's registered
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
