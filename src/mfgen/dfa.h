#ifndef DFA_H
#define DFA_H

#include <stddef.h>
#include <stdint.h>

#include "token.h"

struct dfa {
	uint8_t *alphabet;
	uint8_t alphabet_size;

	uint8_t **trans_indices;
	uint8_t *trans_indices_sizes;

	uint32_t **state_trans;
	uint32_t start;
};

int dfa_init(struct dfa *dfa, struct token *tokens);
int dfa_destroy(struct dfa *dfa);

#endif
