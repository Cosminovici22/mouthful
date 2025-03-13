#ifndef MFLEXER_H
#define MFLEXER_H

#define MFLEXER_ALPHABET_SIZE 128

#define MFLEXER_ERROR_STATE 0
#define MFLEXER_START_STATE 1

struct mflexer {
	unsigned char *alphabet;
	unsigned char **trans_indices;
	unsigned int **trans;
	int *final;
};

#endif
