#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <string>

void protocol_init(uint32_t speed);

size_t protocol_send(uint8_t *buffer, size_t size);

//size_t protocol_receive(uint8_t *buffer, size_t size);
size_t protocol_receive(char *buffer, size_t size);

#endif