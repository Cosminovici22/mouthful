#ifndef ENCODING_H
#define ENCODING_H

#include <stddef.h>

unsigned char *encoding_create(size_t length);
void encoding_destroy(unsigned char *encoding);
size_t encoding_size(unsigned char *encoding);
void encoding_add(unsigned char *encoding, unsigned char byte);

#endif
