#ifndef SESSION_H
#define SESSION_H

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    SESSION_OKAY,
    SESSION_ERROR,
    SESSION_TOGGLE_LED,
    SESSION_TEMPERATURE
} request_t;

bool session_init(void);

request_t session_request(void);

bool session_response(const uint8_t *resp, size_t size);
#endif
