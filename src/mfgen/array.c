#include "array.h"

#include <stddef.h>
#include <stdlib.h>

int array_init(struct array *arr)
{
	if (arr == NULL)
		return 1;

	arr->size = 0;
	arr->elems = NULL;
	arr->_cap = 0;

	return 0;
}

int array_destroy(struct array *arr)
{
	if (arr == NULL)
		return 1;

	free(arr->elems);

	return 0;
}

int array_resize(struct array *arr, size_t size)
{
	if (arr == NULL)
		return 1;

	if (size > arr->_cap) {
		size_t new_cap;
		void **new_elems;

		new_cap = 1;
		// im not sure this isn't UB:
		while ((new_cap <<= 1) < size && new_cap != 0)
			;
		if (new_cap == 0)
			return 1;
		new_elems = realloc(arr->elems, new_cap * sizeof *arr->elems);
		if (new_elems == NULL)
			return 1;

		arr->_cap = new_cap;
		arr->elems = new_elems;
	}
	arr->size = size;

	return 0;
}

int array_push(struct array *arr, void *elem)
{
	if (arr == NULL)
		return 1;

	if (arr->size == arr->_cap) {
		size_t new_cap;
		void **new_elems;

		new_cap = arr->_cap == 0 ? 1 : arr->_cap * 2;
		if (new_cap < arr->_cap)
			return 1;
		new_elems = realloc(arr->elems, new_cap * sizeof *arr->elems);
		if (new_elems == NULL)
			return 1;

		arr->_cap = new_cap;
		arr->elems = new_elems;
	}
	arr->elems[arr->size++] = elem;

	return 0;
}

void *array_pop(struct array *arr)
{
	if (arr == NULL || arr->size == 0)
		return NULL;

	return arr->elems[--arr->size];
}
