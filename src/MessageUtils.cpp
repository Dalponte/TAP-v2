#include "MessageUtils.h"
#include "Atm_tap.h"
#include "Controller.h" // Include Controller.h to get the Command struct definition

const char *MessageUtils::getErrorDescription(MessageErrorCode errorCode)
{
    switch (errorCode)
    {
    case MSG_SUCCESS:
        return "Success";
    case MSG_INVALID_FORMAT:
        return "Invalid message format";
    case MSG_UNKNOWN_COMMAND:
        return "Unknown command type";
    case MSG_JSON_PARSE_ERROR:
        return "JSON parsing error";
    case MSG_INVALID_BINARY_FORMAT:
        return "Invalid binary command format or size"; // Added description
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

Command MessageUtils::parseCommand(const uint8_t *buffer, size_t size)
{
    Command cmd = {0, 0, 0, MSG_SUCCESS}; // Initialize command struct
    const size_t EXPECTED_SIZE = 6;       // 3 * uint16_t

    if (size != EXPECTED_SIZE)
    {
        cmd.errorCode = MSG_INVALID_BINARY_FORMAT;
        return cmd;
    }

    // Extract data respecting Big Endian format
    // tapId = (byte1 << 8) | byte0
    cmd.tapId = ((uint16_t)buffer[0] << 8) | buffer[1];
    // commandType = (byte3 << 8) | byte2
    cmd.commandType = ((uint16_t)buffer[2] << 8) | buffer[3];
    // pulses = (byte5 << 8) | byte4
    cmd.param = ((uint16_t)buffer[4] << 8) | buffer[5];

    // errorCode remains MSG_SUCCESS if we reach here
    return cmd;
}