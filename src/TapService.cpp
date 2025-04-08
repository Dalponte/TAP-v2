#include "TapService.h"
#include "setup.h"

// Initialize static member
TapService *TapService::_instance = nullptr;

TapService::TapService(Atm_led &valve, Atm_led &led, Atm_mqtt_client &mqtt)
    : _valve(valve), _flowmeter(flowmeter), _led(led), _mqtt(mqtt)
{
    _instance = this;
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
        .onPourDone(handlePourDone)
        .onFlowStatus(handleFlowStatus)
        .trace(Serial);
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

// Static callback handlers
void TapService::handleFlow(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_led.trigger(_instance->_led.EVT_TOGGLE);
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

void TapService::handlePourDone(int idx, int v, int up)
{
    if (_instance)
    {
        _instance->_flow_update_timer.stop();

        Serial.print("Pour completed! Pulses poured: ");
        Serial.print(v);
        Serial.print(", Remaining: ");
        Serial.print(up);
        Serial.print(", ID: ");
        Serial.println(_instance->_pour.getCurrentId());
    }
}

void TapService::handleFlowStatus(int idx, int v, int up)
{
    Serial.println();
    Serial.print(up);
    Serial.print(" |  ");
    Serial.print(v);
}
