#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <Arduino.h>
#include "Controller.h"

enum MessageErrorCode
{
    MSG_SUCCESS = 0,
    MSG_INVALID_FORMAT = -2,
    MSG_UNKNOWN_COMMAND = -3,
    MSG_JSON_PARSE_ERROR = -4
};

class MessageUtils
{
public:
    static const char *getErrorDescription(MessageErrorCode errorCode);

    static const char *stateToString(int state);
};

#endif // MESSAGE_UTILS_H