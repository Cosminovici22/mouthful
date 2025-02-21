#include "array.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct array_header {
	size_t length;
	size_t capacity;
	size_t unit;
};

void *array_create(size_t unit)
{
	struct array_header *header;

	header = malloc(sizeof *header);
	if (header == NULL)
		return NULL;

	header->length = 0;
	header->capacity = 0;
	header->unit = unit;

	return header + 1;
}

void array_destroy(void *array)
{
	struct array_header *header;

	header = array - sizeof *header;
	free(header);
}

size_t array_length(void *array)
{
	struct array_header *header;

	header = array - sizeof *header;

	return header->length;
}

void *array_resize(void *array, size_t length)
{
	struct array_header *header;

	header = array - sizeof *header;
	if (length > header->capacity) {
		size_t new_size, new_capacity;

		new_capacity = header->capacity == 0 ? 1 : header->capacity;
		while (new_capacity < length) {
			new_capacity *= 2;
			if (new_capacity < header->capacity) {
				new_capacity = SIZE_MAX;
				break;
			}
		}

		new_size = sizeof *header + new_capacity * header->unit;
		if (new_size < sizeof *header + header->capacity * header->unit)
			return NULL;
		header = realloc(header, new_size);
		if (header == NULL)
			return NULL;

		header->capacity = new_capacity;
		array = header + 1;
	}
	header->length = length;

	return array;
}

void *array_push(void *array, void *elem)
{
	struct array_header *header;

	array = array_resize(array, array_length(array) + 1);
	if (array == NULL)
		return NULL;

	header = array - sizeof *header;
	memcpy(array + (header->length - 1) * header->unit, elem, header->unit);

	return array;
}

void *array_pop(void *array)
{
	struct array_header *header;

	header = array - sizeof *header;
	if (header->length == 0)
		return NULL;
	header->length--;

	return array + header->length * header->unit;
}
