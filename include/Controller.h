#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoMqttClient.h>
#include "Atm_mqtt_client.h"
#include "LedService.h"
#include "Atm_tap.h"

// Structure for tap configuration
struct TapConfig
{
    uint16_t tapId;
    const char *tapName;
};

// Define structure for parsed JSON command
struct JsonCommand
{
    uint16_t tapId;
    uint8_t commandType;
    uint16_t pulses;
    bool isValid; // Flag to indicate if parsing was successful
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

    // Setup method for initial configuration
    static void setup();

    // MQTT message handling
    static void handleMqttMessage(int messageSize);

    // Add processJsonCommand declaration
    static void processJsonCommand(const JsonCommand &command);

    // Error handling method
    static void handleError(const char *errorMessage);

    // Add parseJsonMessage declaration
    static JsonCommand parseJsonMessage(const uint8_t *buffer, size_t length);

    // Connection event callbacks
    static void onConnected(int idx, int v, int up);
    static void onDisconnected(int idx, int v, int up);

    // Add the state change publish handler
    static void publishTapStateChanged(int idx, int state, int up);

    // Tap state callback handlers
    static void onTapInitializing(int idx, int v, int up);
    static void onTapReady(int idx, int v, int up);
    static void onTapPouring(int idx, int v, int up);
    static void onTapDone(int idx, int v, int up);
    static void onTapDisconnected(int idx, int v, int up);

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
};

#endif // CONTROLLER_H
