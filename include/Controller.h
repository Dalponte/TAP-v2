#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoMqttClient.h>
#include "Atm_mqtt_client.h"
#include "LedService.h"
#include "Atm_tap.h"
#include "MessageUtils.h" // Ensure MessageUtils is included

// Define pin constants
#define FLOWMETER_PIN 21 // Flowmeter button simulation pin
#define VALVE_PIN 22     // Mosfet valve simulation pin

// Define timing constants
#define FLOW_UPDATE_INTERVAL_MS 500

// Structure for tap configuration
struct TapConfig
{
    uint16_t tapId;
    const char *tapName;
};

// Define structure for parsed JSON command
struct Command
{
    uint16_t tapId;
    uint16_t commandType;
    uint16_t param;
    MessageErrorCode errorCode; // Add error code field
};

// Command types
enum CommandType
{
    CMD_POUR = 1,
    CMD_CONTINUE = 2,
};

class Controller
{
public:
    // Get the existing instance (singleton)
    static Controller &getInstance();

    // Initialize with required parameters
    static Controller &initInstance(LedService &led, const TapConfig &config);

    // Initialize MQTT connection
    void begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId = "client_id");

    // MQTT publish functionality
    void publish(const char *topic, const char *payload);

    // MQTT message handling
    static void handleMqttMessage(int messageSize);

    // Add processJsonCommand declaration
    static void processCommand(const Command &command);

    // Error handling method
    static void handleError(const char *errorMessage);

    // Connection event callbacks
    static void onConnected(int idx, int v, int up);
    static void onDisconnected(int idx, int v, int up);

    // Add the state change publish handler
    static void publishTapStateChanged(int idx, int state, int up);

    // Tap state callback handlers
    static void onTapDone(int idx, int v, int up);

    // Public attribute for tap machine
    Atm_tap _tap;

private:
    // Private constructor for singleton
    Controller(LedService &led, const TapConfig &config);

    // No copy or assignment
    Controller(const Controller &) = delete;
    Controller &operator=(const Controller &) = delete;

    // Configuration
    TapConfig _config;

    // MQTT components
    EthernetClient _ethClient;
    MqttClient _mqttClient{_ethClient};
    Atm_mqtt_client _mqtt;

    // Buffer for message storage
    uint8_t _messageBuffer[256];

    // Static references
    static LedService *ledService;
    static Controller *_instance;

    // Flowmeter and valve components
    Atm_digital _flowmeter;
    Atm_timer _flow_update_timer;
    Atm_led _valve; // Valve control
    
    // Flow update handler
    static void handleFlowUpdateTimer(int idx, int v, int up);
    static void handleFlow(int idx, int v, int up);
};

#endif // CONTROLLER_H
