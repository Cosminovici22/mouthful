#include "array.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct array_header {
	size_t unit;
	size_t length;
	size_t capacity;
};

static inline struct array_header *header_of(void *array)
{
	return array - sizeof(struct array_header);
}

static inline int add_wraps(size_t a, size_t b)
{
	return SIZE_MAX - b < a;
}

static inline int mul_wraps(size_t a, size_t b)
{
	return a * b / a != b;
}

static void *
header_alloc(struct array_header *header, size_t unit, size_t capacity)
{
	size_t size;

	if (mul_wraps(capacity, unit))
		return NULL;
	size = capacity * unit;

	if (add_wraps(size, sizeof *header))
		return NULL;
	size += sizeof *header;

	header = realloc(header, size);
	if (header == NULL)
		return NULL;
	header->capacity = capacity;

	return header;
}

void *array_create(size_t length, size_t unit)
{
	struct array_header *header;

	header = header_alloc(NULL, unit, length);
	if (header == NULL)
		return NULL;

	header->unit = unit;
	header->length = length;

	return header + 1;
}

void array_destroy(void *array)
{
	struct array_header *header;

	header = header_of(array);
	free(header);
}

size_t array_length(void *array)
{
	struct array_header *header;

	header = header_of(array);

	return header->length;
}

void *array_resize(void *array, size_t length)
{
	struct array_header *header;

	header = header_of(array);
	if (length > header->capacity) {
		header = header_alloc(header, header->unit, length);
		if (header == NULL)
			return NULL;

		array = header + 1;
	}
	header->length = length;

	return array;
}

void *array_push(void *array, void *elem)
{
	struct array_header *header;

	header = header_of(array);
	if (header->length == header->capacity) {
		size_t capacity;

		if (header->capacity == 0)
			capacity = 1;
		else if (!mul_wraps(2, header->capacity))
			capacity = 2 * header->capacity;
		else
			return NULL;

		header = header_alloc(header, header->unit, capacity);
		if (header == NULL)
			return NULL;

		array = header + 1;
	}
	memcpy(array + header->length * header->unit, elem, header->unit);
	header->length++;

	return array;
}

void *array_pop(void *array)
{
	struct array_header *header;

	header = header_of(array);
	if (header->length == 0)
		return NULL;
	header->length--;

	return array + header->length * header->unit;
}
