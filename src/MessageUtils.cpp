#include "MessageUtils.h"
#include "Atm_tap.h"

MessageErrorCode MessageUtils::parseBinaryMessage(const uint8_t *buffer, size_t length, BinaryMessage &message)
{
    // Check minimum message size (1+2+1+2+2+2=10 bytes)
    if (length < 10)
    {
        Serial.println("Binary message too short");
        return MSG_TOO_SHORT;
    }

    // Extract fields from binary message
    size_t pos = 0;

    // Read protocol version (1 byte)
    message.protocolVersion = buffer[pos++];

    // Read tap ID (2 bytes, big-endian)
    message.tapId = (buffer[pos] << 8) | buffer[pos + 1];
    pos += 2;

    // Read command type (1 byte)
    message.commandType = buffer[pos++];

    // Read param1 (2 bytes, big-endian)
    message.param1 = (buffer[pos] << 8) | buffer[pos + 1];
    pos += 2;

    // Read param2 (2 bytes, big-endian)
    message.param2 = (buffer[pos] << 8) | buffer[pos + 1];
    pos += 2;

    // Read param3 (2 bytes, big-endian)
    message.param3 = (buffer[pos] << 8) | buffer[pos + 1];

    return MSG_SUCCESS;
}

MessageErrorCode MessageUtils::validateMessage(const BinaryMessage &message, uint16_t tapId)
{
    // Check if the message is for this tap
    if (message.tapId != tapId)
    {
        return MSG_WRONG_TAP_ID;
    }

    // Validate command type
    switch (message.commandType)
    {
    case CMD_POUR:
    case CMD_STOP:
    case CMD_STATUS:
        return MSG_SUCCESS;
    default:
        return MSG_UNKNOWN_COMMAND;
    }
}

const char *MessageUtils::getErrorDescription(MessageErrorCode errorCode)
{
    switch (errorCode)
    {
    case MSG_SUCCESS:
        return "Success";
    case MSG_TOO_SHORT:
        return "Binary message too short";
    case MSG_INVALID_FORMAT:
        return "Invalid message format";
    case MSG_WRONG_TAP_ID:
        return "Binary message not for this tap";
    case MSG_UNKNOWN_COMMAND:
        return "Unknown command type";
    default:
        return "Undefined error";
    }
}

const char *MessageUtils::stateToString(int state)
{
    switch (state)
    {
    case Atm_tap::INITIALIZING:
        return "INITIALIZING";
    case Atm_tap::READY:
        return "READY";
    case Atm_tap::POURING:
        return "POURING";
    case Atm_tap::DONE:
        return "DONE";
    case Atm_tap::DISCONNECTED:
        return "DISCONNECTED";
    default:
        return "UNKNOWN";
    }
}