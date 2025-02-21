#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

struct array {
	size_t size;
	void **elems;
	size_t _cap;
};

int array_init(struct array *arr);
int array_destroy(struct array *arr);
int array_resize(struct array *arr, size_t size);
int array_push(struct array *arr, void *elem);
void *array_pop(struct array *arr);

#endif
