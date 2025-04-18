# TAP-v2 (Beer Tap Management System)

TAP-v2 is an Arduino-based beer tap management system built on the Automaton framework, designed to control and monitor beer taps over MQTT. The system combines reactive programming principles with IoT capabilities to deliver a reliable and modular solution for beer pouring management.

## Overview

This project uses the Automaton framework to create a state-machine based tap control system that communicates with a central server via MQTT. Each tap controller is responsible for:

- Managing the physical beer tap hardware
- Reporting pour events and status updates
- Responding to pour requests from the network
- Providing visual feedback through LEDs

## Hardware Requirements

- Arduino-compatible board with Ethernet support
- Ethernet shield or built-in Ethernet
- Buttons/switches for manual control
- LEDs for status indication
- Flow sensors and valves (for beer measurement and control)

## MQTT Communication

The TAP-v2 system uses MQTT for all network communications. Below is the documentation for the MQTT topics and payloads used:

### Topics Structure

| Topic        | Direction | Description                            |
|--------------|-----------|----------------------------------------|
| `tap/command`| Incoming  | Commands sent to the tap controller    |
| `tap/input`  | Outgoing  | Messages sent from the tap controller  |
| `tap/state`  | Outgoing  | State updates from the tap controller  |

### Incoming Messages (`tap/in`)

The system accepts binary messages on the `tap/in` topic. Messages are structured as binary data with the following format:

**Binary Message Format:**
- Protocol Version (1 byte): Version of the protocol
- Tap ID (2 bytes): ID of the target tap
- Command Type (1 byte): Type of command to execute
- Parameter 1 (2 bytes): Command-specific parameter (e.g., pour amount)
- Parameter 2 (2 bytes): Reserved for future use
- Parameter 3 (2 bytes): Reserved for future use

**Supported Commands:**

| Command Type | Value | Description                   | Parameters                        |
|--------------|-------|-------------------------------|-----------------------------------|
| CMD_POUR     | 1     | Initiates a pour operation    | param1: Amount in pulses/ml       |
| CMD_STOP     | 2     | Stops an active pour          | None                              |
| CMD_STATUS   | 3     | Requests current tap status   | None                              |

### Outgoing Messages

#### `tap/input` Topic
Used for general messages and notifications from the tap controller:
- Button press notifications
- Error messages
- General status information

#### `tap/state` Topic
Used specifically for state change notifications. The payload is a JSON object with the following structure:

```json
{
  "state": "[STATE_NAME]",
  "id": [TAP_ID],
  "name": "[TAP_NAME]"
}
```

### Tap States

The tap controller implements a state machine with the following states that are published over MQTT:

| State           | Description                                           |
|-----------------|-------------------------------------------------------|
| `INITIALIZING`  | Startup state, establishing network and MQTT connections |
| `READY`         | Tap is connected and ready to accept pour requests    |
| `POURING`       | Actively pouring beer                                 |
| `DONE`          | Completed a pour operation                            |
| `DISCONNECTED`  | Lost connection to MQTT broker                        |

### MQTT Connection Details

The system connects to an MQTT broker with the following default settings:
- Broker address: 192.168.4.2
- Port: 1883
- Client ID: "client_id" (configurable)
- Default IP: 192.168.4.100

## State Transitions

The tap state machine transitions between states based on the following events:

- `EVT_CONNECTED`: Transitions from INITIALIZING or DISCONNECTED to READY
- `EVT_POUR`: Transitions from READY to POURING
- `EVT_STOP`: Transitions from POURING to DONE
- `EVT_READY`: Transitions from DONE to READY
- `EVT_DISCONNECT`: Transitions to DISCONNECTED from any state

## Development

This project is built using the Automaton framework, which provides a reactive programming model for Arduino. Key features of the implementation include:

- Event-driven architecture with state machines
- Singleton pattern for service management
- Binary protocol for efficient communication
- JSON formatted state updates for easy parsing

### Automaton Framework

The project heavily relies on the Automaton framework, which facilitates creating:
- State machines (extending the Machine class)
- Event-driven programming
- Component communication through triggers and callbacks

## Building and Uploading

The project uses PlatformIO for development. To build and upload:

1. Install PlatformIO
2. Clone this repository
3. Open the project in PlatformIO
4. Configure your network settings in `main.cpp`
5. Build and upload to your Arduino board

## License

[Your license information here]
