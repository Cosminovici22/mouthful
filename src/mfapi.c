#include "../include/mfapi.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/mflexer.h"

static long file_size(FILE *file)
{
	long old_pos, new_pos, ret;

	old_pos = ftell(file);
	if (old_pos < 0)
		return -1;

	ret = fseek(file, 0, SEEK_END);
	if (ret < 0)
		return -1;

	new_pos = ftell(file);
	if (new_pos < 0)
		return -1;

	ret = fseek(file, old_pos, SEEK_SET);
	if (ret < 0)
		return -1;

	return new_pos - old_pos;
}

int mflexer_init(struct mflexer *lexer, FILE *file)
{
	size_t ret, alphabet_size, state_count, *sizes;
	long lexer_size;

	ret = fread(&alphabet_size, sizeof alphabet_size, 1, file);
	if (ret != 1)
		return -1;

	ret = fread(&state_count, sizeof state_count, 1, file);
	if (ret != 1)
		return -1;

	sizes = malloc(state_count * sizeof(size_t));
	if (sizes == NULL)
		return -1;

	ret = fread(sizes, sizeof *sizes, state_count, file);
	if (ret != state_count)
		return -1;

	lexer_size = file_size(file);
	if (lexer_size < 0)
		return -1;

	lexer->alphabet = malloc(lexer_size);
	if (lexer->alphabet < 0)
		return -1;

	ret = fread(lexer->alphabet, 1, lexer_size, file);
	if (ret < 0)
		return -1;

	lexer->trans_indices = malloc(alphabet_size
		* sizeof *lexer->trans_indices);
	if (lexer->trans_indices == NULL)
		return -1;

	lexer->trans_indices[0] = (void *) lexer->alphabet
		+ MFLEXER_ALPHABET_SIZE * sizeof *lexer->alphabet;
	for (size_t i = 1; i < alphabet_size; i++)
		lexer->trans_indices[i] = (void *) lexer->trans_indices[i - 1]
			+ alphabet_size * sizeof *lexer->alphabet;

	lexer->trans = malloc(state_count * sizeof *lexer->trans);
	if (lexer->trans == NULL)
		return -1;

	lexer->trans[0] = (void *) lexer->trans_indices[alphabet_size - 1]
		+ alphabet_size * sizeof *lexer->alphabet;
	for (size_t i = 1; i < state_count; i++)
		lexer->trans[i] = (void *) lexer->trans[i - 1] + sizes[i - 1]
			* sizeof **lexer->trans;

	lexer->final = (void *) lexer->trans[state_count - 1]
		+ sizes[state_count - 1] * sizeof **lexer->trans;

	free(sizes);

	return 0;
}

void mflexer_destroy(struct mflexer *lexer)
{
	free(lexer->trans_indices);
	free(lexer->trans);
	free(lexer->alphabet);
}

int mflexer_tokenize(const struct mflexer *lexer, FILE *file)
{
	unsigned int state;
	unsigned char prev_symbol;
	int type, ret;

	state = MFLEXER_START_STATE;
	prev_symbol = lexer->alphabet[0];
	while (state != MFLEXER_ERROR_STATE) {
		unsigned char byte, symbol, index;

		type = lexer->final[state];

		ret = getc(file);
		if (ret < 0)
			return feof(file) ? type : MFAPI_ERROR;

		byte = ret;
		symbol = lexer->alphabet[byte];
		index = lexer->trans_indices[prev_symbol][symbol];

		state = lexer->trans[state][index];
		prev_symbol = symbol;
	}

	ret = ungetc(ret, file);
	if (ret < 0)
		return MFAPI_ERROR;

	return type;
}
