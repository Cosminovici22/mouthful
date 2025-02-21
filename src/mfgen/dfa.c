#include "dfa.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void dfa_alphabet_init(struct dfa *dfa, struct array *tokens)
{
	dfa->alphabet_size = 1;
	memset(dfa->alphabet, 0, sizeof dfa->alphabet);

	for (size_t i = 0; i < tokens->size; i++) {
		struct token *curr_token;

		curr_token = tokens->elems[i];
		for (size_t j = 0; curr_token->value[j] != '\0'; j++) {
			uint8_t curr_chr;

			curr_chr = curr_token->value[j];
			if (dfa->alphabet[curr_chr] == 0)
				dfa->alphabet[curr_chr] = dfa->alphabet_size++;
		}
	}
}


static int dfa_trans_indices_init(struct dfa *dfa, struct array *tokens)
{
	size_t aux;

	aux = dfa->alphabet_size + 1;

	dfa->trans_indices_sizes = malloc(aux * sizeof *dfa->trans_indices);
	if (dfa->trans_indices_sizes == NULL)
		return 1;
	for (size_t i = 0; i < aux; i++)
		dfa->trans_indices_sizes[i] = 1;

	dfa->trans_indices = malloc(aux * sizeof *dfa->trans_indices);
	if (dfa->trans_indices == NULL)
		return 1;

	for (size_t i = 0; i < aux; i++) {
		dfa->trans_indices[i] = calloc(dfa->alphabet_size, sizeof *dfa->trans_indices);
		if (dfa->trans_indices[i] == NULL)
			return 1;
	}

	for (size_t i = 0; i < tokens->size; i++) {
		struct token *curr_token;
		size_t prev_symbol;

		curr_token = tokens->elems[i];
		prev_symbol = dfa->alphabet_size;
		for (size_t j = 0; curr_token->value[j] != '\0'; j++) {
			uint8_t curr_chr, curr_symbol;

			curr_chr = curr_token->value[j];
			curr_symbol = dfa->alphabet[curr_chr];
			if (dfa->trans_indices[prev_symbol][curr_symbol] == 0)
				dfa->trans_indices[prev_symbol][curr_symbol] = dfa->trans_indices_sizes[prev_symbol]++;
			prev_symbol = curr_symbol;
		}
	}

	return 0;
}

int dfa_insert_token(struct dfa *dfa, struct token *token)
{
	size_t state, prev_symbol;

	state = dfa->start;
	prev_symbol = dfa->alphabet_size;
	for (size_t i = 0; token->value[i] != '\0'; i++) {
		uint8_t curr_chr, curr_trans;
		size_t curr_symbol;
		uint32_t **state_trans;

		curr_chr = token->value[i];
		curr_symbol = dfa->alphabet[curr_chr];
		curr_trans = dfa->trans_indices[prev_symbol][curr_symbol];

		state_trans = (uint32_t **) dfa->state_trans.elems;
		if (state_trans[state][curr_trans] == 0) {
			uint32_t *new_state;

			new_state = calloc(dfa->trans_indices_sizes[curr_symbol], sizeof *new_state);
			if (new_state == NULL)
				return 1;
			array_push(&dfa->state_trans, new_state);
			array_push(&dfa->state_trans_sizes, &dfa->trans_indices_sizes[curr_symbol]);

			state_trans = (uint32_t **) dfa->state_trans.elems;
			state_trans[state][curr_trans] = dfa->state_trans.size - 1;
		}

		state = state_trans[state][curr_trans];
		prev_symbol = curr_symbol;
	}
	// set state as final

	return 0;
}

int dfa_init(struct dfa *dfa, struct array *tokens)
{
	int ret;
	uint32_t *error_state, *start_state;

	dfa_alphabet_init(dfa, tokens);
	ret = dfa_trans_indices_init(dfa, tokens);
	if (ret != 0)
		return 1;

	dfa->start = 1;
	array_init(&dfa->state_trans);
	array_init(&dfa->state_trans_sizes);

	error_state = calloc(dfa->trans_indices_sizes[0], sizeof *error_state);
	if (error_state == NULL)
		return 1;
	array_push(&dfa->state_trans, error_state);
	array_push(&dfa->state_trans_sizes, &dfa->trans_indices_sizes[0]);

	start_state = calloc(dfa->trans_indices_sizes[dfa->alphabet_size], sizeof *error_state);
	if (start_state == NULL)
		return 1;
	array_push(&dfa->state_trans, start_state);
	array_push(&dfa->state_trans_sizes, &dfa->trans_indices_sizes[dfa->alphabet_size]);

	for (size_t i = 0; i < tokens->size; i++) {
		struct token *curr_token;

		curr_token = tokens->elems[i];
		ret = dfa_insert_token(dfa, curr_token);
		if (ret != 0) // cleanup..
			return 1;
	}

	return 0;
}

int dfa_destroy(struct dfa *dfa)
{
	return 0;
}
