#include"ser.h"
#include <Arduino.h>
#include "protocol.h"

enum
{
    STATUS_OKAY,
    STATUS_ERROR
};

enum
{
    SESSION_CLOSE,
    SESSION_GET_TEMP,
    SESSION_TOGGLE_LED,
    SESSION_ESTABLISH
};

static uint64_t session_id = 0;

void ser_init(void)
{
    protocol_init();
    pinMode(GPIO_NUM_21, OUTPUT);
    digitalWrite(GPIO_NUM_21, LOW);
}

void ser_run(void)
{
    uint8_t buffer[16] = {0};
    size_t len = protocol_receive(buffer, sizeof(buffer));

    if (len == 1)
    {
        switch (buffer[0])
        {
        case SESSION_CLOSE:
            session_id = 0;
            buffer[0] = STATUS_OKAY;
            protocol_send(buffer, 1);
            break;

        case SESSION_ESTABLISH:
            buffer[0] = STATUS_OKAY;
            for (int i = 0; i < sizeof(session_id); i++)
            {
                uint8_t *ptr = (uint8_t *)(&session_id);
                ptr[i] = random(1, 256);
            }
            memcpy(buffer + 1, &session_id, sizeof(session_id));
            protocol_send(buffer, sizeof(session_id) + 1);
            break;

        case SESSION_GET_TEMP:
        {
            buffer[0] = STATUS_OKAY;
            float temperature = temperatureRead();
            memcpy(buffer + 1, &temperature, sizeof(float));
            protocol_send(buffer, sizeof(float) + 1);
        }
        break;

        case SESSION_TOGGLE_LED:
        {
            static uint8_t ledState = LOW;
            ledState = !ledState;
            digitalWrite(GPIO_NUM_21, ledState);

            buffer[1] = digitalRead(GPIO_NUM_21);
            buffer[0] = (buffer[1] == ledState) ? STATUS_OKAY : STATUS_ERROR;
            protocol_send(buffer, 2);
        }
        break;

        default:
            buffer[0] = STATUS_ERROR;
            protocol_send(buffer, 1);
            break;
        }
    }
    else
    {
        buffer[0] = STATUS_ERROR;
        protocol_send(buffer, 1);
    }
}