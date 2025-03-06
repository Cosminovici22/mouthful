#ifndef ENCODING_H
#define ENCODING_H

#include <stddef.h>

typedef unsigned char byte_t;

byte_t *encoding_create(size_t length);
void encoding_destroy(byte_t *encoding);
size_t encoding_size(byte_t *encoding);
void encoding_add(byte_t *encoding, byte_t byte);

#endif
