#include "encoding.h"

#include <limits.h>
#include <stdlib.h>

unsigned char *encoding_create(size_t length)
{
	unsigned char *header;

	if (length > UCHAR_MAX + 1)
		return NULL;

	header = calloc(length + 1, sizeof *header);
	if (header == NULL)
		return NULL;

	return header + 1;
}

void encoding_destroy(unsigned char *encoding)
{
	free(encoding - 1);
}

size_t encoding_size(unsigned char *encoding)
{
	return encoding[-1] + 1;
}

void encoding_add(unsigned char *encoding, unsigned char byte)
{
	if (encoding[byte] != 0 || encoding[-1] == UCHAR_MAX)
		return;

	encoding[byte] = ++encoding[-1];
}
