#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

void *array_create(size_t length, size_t unit);
void array_destroy(void *array);
size_t array_length(const void *array);
void *array_resize(void *array, size_t length);
void *array_push(void *array, void *elem);
void *array_pop(void *array);

#endif
