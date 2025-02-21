#ifndef DFA_H
#define DFA_H

#include <stddef.h>
#include <stdint.h>

#include "array.h"
#include "token.h"

struct dfa {
	uint8_t alphabet[256];
	uint8_t alphabet_size;

	uint8_t **trans_indices;
	uint8_t *trans_indices_sizes;

	size_t start;
	struct array state_trans;
	struct array state_trans_sizes;
};

int dfa_init(struct dfa *dfa, struct array *tokens);
int dfa_destroy(struct dfa *dfa);

#endif
