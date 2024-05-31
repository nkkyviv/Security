#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

bool protocol_init();

size_t protocol_send(const uint8_t *buffer, size_t size);

size_t protocol_receive(uint8_t *buffer, size_t size);

#endif