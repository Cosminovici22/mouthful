#include "dfa.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

static int dfa_alphabet_init(struct dfa *dfa, struct token *tokens)
{
	size_t tokens_length;

	dfa->alphabet_size = 1;
	dfa->alphabet = calloc(256, sizeof *dfa->alphabet);
	if (dfa->alphabet == NULL)
		return 1;

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		struct token *curr_token;

		curr_token = &tokens[i];
		for (size_t j = 0; curr_token->value[j] != '\0'; j++) {
			uint8_t curr_chr;

			curr_chr = curr_token->value[j];
			if (dfa->alphabet[curr_chr] == 0)
				dfa->alphabet[curr_chr] = dfa->alphabet_size++;
		}
	}

	return 0;
}


static int dfa_trans_indices_init(struct dfa *dfa, struct token *tokens)
{
	size_t aux, tokens_length;

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

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		struct token *curr_token;
		size_t prev_symbol;

		curr_token = &tokens[i];
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

static int dfa_insert_token(struct dfa *dfa, struct token *token)
{
	size_t state, prev_symbol;

	state = dfa->start;
	prev_symbol = dfa->alphabet_size;
	for (size_t i = 0; token->value[i] != '\0'; i++) {
		uint8_t curr_chr, curr_trans;
		size_t curr_symbol;

		curr_chr = token->value[i];
		curr_symbol = dfa->alphabet[curr_chr];
		curr_trans = dfa->trans_indices[prev_symbol][curr_symbol];

		if (dfa->state_trans[state][curr_trans] == 0) {
			uint32_t *new_state;

			// error check
			new_state = array_create(sizeof *new_state);
			new_state = array_resize(new_state, dfa->trans_indices_sizes[curr_symbol]);
			memset(new_state, 0, sizeof *new_state * array_length(new_state));
			dfa->state_trans = array_push(dfa->state_trans, &new_state);

			dfa->state_trans[state][curr_trans] = array_length(dfa->state_trans) - 1;
		}

		state = dfa->state_trans[state][curr_trans];
		prev_symbol = curr_symbol;
	}
	// set state as final

	return 0;
}

int dfa_init(struct dfa *dfa, struct token *tokens)
{
	uint32_t *error_state, *start_state;
	size_t tokens_length;
	int ret;

	ret = dfa_alphabet_init(dfa, tokens);
	if (ret != 0)
		return 1;

	ret = dfa_trans_indices_init(dfa, tokens);
	if (ret != 0)
		return 1;

	dfa->state_trans = array_create(sizeof *dfa->state_trans);
	dfa->start = 1;

	// error check
	error_state = array_create(sizeof *error_state);
	error_state = array_resize(error_state, dfa->trans_indices_sizes[0]);
	memset(error_state, 0, sizeof *error_state * array_length(error_state));
	dfa->state_trans = array_push(dfa->state_trans, &error_state);

	// error check
	start_state = array_create(sizeof *start_state);
	start_state = array_resize(start_state, dfa->trans_indices_sizes[dfa->alphabet_size]);
	memset(start_state, 0, sizeof *start_state * array_length(start_state));
	dfa->state_trans = array_push(dfa->state_trans, &start_state);

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		struct token *curr_token;

		curr_token = &tokens[i];
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
