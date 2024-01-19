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

size_t protocol_send(uint8_t *buffer, size_t size)
{
    return UART.write(buffer, size);
}



// size_t protocol_receive(uint8_t *buffer, size_t size)
// {
//   return UART.readBytes(buffer, size);
// }
size_t protocol_receive(char *buffer, size_t size)
{
  return UART.readBytes(buffer, size);
}