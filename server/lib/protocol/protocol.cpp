/*using serial communication use serial0 to connect server and client*/
#include "protocol.h"
#include <Arduino.h>

#define UART Serial

bool status = false;

void protocol_init(uint32_t speed)
{
    UART.begin(speed);
    delay(2000);
}

void protocol_send(uint8_t *buffer, size_t size)
{
    UART.write(buffer, size);
}

bool protocol_receive(uint8_t *buffer, size_t size)
{
    status = false;

    size_t readBuffer = UART.readBytes(buffer, size);

    if (readBuffer > 0)
    {
        status = true;
    }

    return status;
}
