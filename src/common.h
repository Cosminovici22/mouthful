#ifndef COMMON_H
#define COMMON_H

#define MFLEXER_ALPHABET_SIZE 128
#define MFLEXER_ERROR_STATE 0
#define MFLEXER_START_STATE 1
#define MFLEXER_INVALID_TOK -1

typedef unsigned char mfbyte_t;
typedef unsigned int mfstate_t;

struct mflexer {
	mfbyte_t *alphabet;
	mfbyte_t **trans_indices;
	mfstate_t **trans;
	int *final;
};

enum {
	MFSTATUS_SUCCESS = 0,
	MFSTATUS_ERR_TOK = MFLEXER_INVALID_TOK,
	MFSTATUS_ERR_IO = -2,
	MFSTATUS_ERR_MEM = -3
};

#endif
