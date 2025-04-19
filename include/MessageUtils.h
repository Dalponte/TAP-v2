#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <Arduino.h>
// Forward declare Command struct to avoid circular dependency if Controller.h includes this
struct Command;

enum MessageErrorCode
{
    MSG_SUCCESS = 0,
    MSG_INVALID_FORMAT = -2,
    MSG_UNKNOWN_COMMAND = -3,
    MSG_JSON_PARSE_ERROR = -4,
    MSG_INVALID_BINARY_FORMAT = -5
};

class MessageUtils
{
public:
    static const char *getErrorDescription(MessageErrorCode errorCode);

    static const char *stateToString(int state);

    static Command parseCommand(const uint8_t *buffer, size_t size);
};

#endif // MESSAGE_UTILS_H