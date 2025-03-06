#include "encoding.h"

#include <limits.h>
#include <stdlib.h>

byte_t *encoding_create(size_t length)
{
	byte_t *header;

	if (length > UCHAR_MAX + 1)
		return NULL;

	header = calloc(length + 1, sizeof *header);
	if (header == NULL)
		return NULL;

	return header + 1;
}

void encoding_destroy(byte_t *encoding)
{
	free(encoding - 1);
}

size_t encoding_size(byte_t *encoding)
{
	return encoding[-1] + 1;
}

void encoding_add(byte_t *encoding, byte_t byte)
{
	if (encoding[byte] != 0 || encoding[-1] == UCHAR_MAX)
		return;

	encoding[byte] = ++encoding[-1];
}
