#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define ASSERT(assertion, ...) \
	if (!(assertion)) { \
		fprintf(stderr, __VA_ARGS__); \
		exit(EXIT_FAILURE); \
	}

#define STATUS_STRING(status) \
	status == MFSTATUS_SUCCESS \
		? "Success" \
		: status == MFSTATUS_ERR_TOK \
		? "Tokenization error" \
		: status == MFSTATUS_ERR_IO \
		? "I/O error" \
		: status == MFSTATUS_ERR_MEM \
		? "Allocation error" \
		: ""

#endif
