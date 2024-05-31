#include "protocol.h"
#include <Arduino.h>

#define BAUDRATE 115200

bool protocol_init()
{
  Serial.begin(BAUDRATE);
  return Serial;
}

size_t protocol_send(const uint8_t *buffer, size_t size)
{
  return Serial.write(buffer, size);
}

size_t protocol_receive(uint8_t *buffer, size_t size)
{

  while (0 == Serial.available())
  {
    ;
  }

  return Serial.readBytes(buffer, size);
}