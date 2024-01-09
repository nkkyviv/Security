#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

void protocol_init(uint32_t speed);

void protocol_send(uint8_t *buffer, size_t size);

bool protocol_receive(uint8_t *buffer, size_t size);

#endif