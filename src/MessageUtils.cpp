#include "MessageUtils.h"
#include "Atm_tap.h"

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