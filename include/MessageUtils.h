#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <Arduino.h>
#include "Controller.h"

// Error codes for message handling
enum MessageErrorCode
{
    MSG_SUCCESS = 0,
    MSG_TOO_SHORT = -1,
    MSG_INVALID_FORMAT = -2,
    MSG_WRONG_TAP_ID = -3,
    MSG_UNKNOWN_COMMAND = -4
};

// Class for handling message parsing and validation
class MessageUtils
{
public:
    // Parse binary message and return error code
    static MessageErrorCode parseBinaryMessage(const uint8_t *buffer, size_t length, BinaryMessage &message);

    // Validate message for a specific tap ID
    static MessageErrorCode validateMessage(const BinaryMessage &message, uint16_t tapId);

    // Get error description for an error code
    static const char *getErrorDescription(MessageErrorCode errorCode);

    // Convert state to string
    static const char *stateToString(int state);
};

#endif // MESSAGE_UTILS_H