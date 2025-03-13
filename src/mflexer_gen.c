#include "mflexer_gen.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../include/mflexer.h"
#include "array.h"
#include "encoding.h"

struct token {
	unsigned int type;
	char *value;
};

static int read_tokens(FILE *file, struct token **tokens)
{
	while (!feof(file)) {
		struct token token, *temp;
		int ret;

		ret = fscanf(file, " %9u : %ms ", &token.type, &token.value);
		if (ret != 2) // might not work
			return -1;

		temp = array_push(*tokens, &token);
		if (temp == NULL)
			return -1;
		*tokens = temp;
	}

	return 0;
}

static int
mflexer_alphabet_init(struct mflexer *lexer, const struct token *tokens)
{
	size_t tokens_length;

	lexer->alphabet = encoding_create(MFLEXER_ALPHABET_SIZE);
	if (lexer->alphabet == NULL)
		return -1;

	tokens_length = array_length(tokens);
	encoding_add(lexer->alphabet, 0);
	for (size_t i = 0; i < tokens_length; i++)
		for (size_t j = 0; tokens[i].value[j] != '\0'; j++) {
			unsigned char byte;

			byte = tokens[i].value[j];
			encoding_add(lexer->alphabet, byte);
		}

	return 0;
}

static int
mflexer_trans_indices_init(struct mflexer *lexer, const struct token *tokens)
{
	size_t temp, tokens_length;

	temp = encoding_size(lexer->alphabet);

	lexer->trans_indices = array_create(temp, sizeof *lexer->trans_indices);
	if (lexer->trans_indices == NULL)
		return -1;

	for (size_t i = 0; i < temp; i++) {
		lexer->trans_indices[i] = encoding_create(temp);
		if (lexer->trans_indices[i] == NULL)
			return -1;
	}

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		unsigned char prev_symbol;

		prev_symbol = lexer->alphabet[0];
		for (size_t j = 0; tokens[i].value[j] != '\0'; j++) {
			unsigned char byte, symbol;

			byte = tokens[i].value[j];
			symbol = lexer->alphabet[byte];
			encoding_add(lexer->trans_indices[prev_symbol], symbol);

			prev_symbol = symbol;
		}
	}

	return 0;
}

static int
mflexer_create_state(struct mflexer *lexer, size_t indice_count)
{
	unsigned int *indices;
	int type;

	indices = array_create(indice_count, sizeof *indices);
	if (indices == NULL)
		return -1;
	memset(indices, MFLEXER_ERROR_STATE
		, sizeof *indices * array_length(indices));

	lexer->trans = array_push(lexer->trans, &indices);
	if (lexer->trans == NULL)
		return -1;

	type = -1;
	lexer->final = array_push(lexer->final, &type);
	if (lexer->final == NULL)
		return -1;

	return 0;
}

static int
mflexer_insert_token(struct mflexer *lexer, const struct token *token)
{
	unsigned int state;
	unsigned char prev_symbol;

	state = MFLEXER_START_STATE;
	prev_symbol = lexer->alphabet[0];
	for (size_t i = 0; token->value[i] != '\0'; i++) {
		unsigned char byte, symbol, index;

		byte = token->value[i];
		symbol = lexer->alphabet[byte];
		index = lexer->trans_indices[prev_symbol][symbol];

		if (lexer->trans[state][index] == 0) {
			int ret;

			ret = mflexer_create_state(lexer
				, encoding_size(lexer->trans_indices[symbol]));
			if (ret < 0)
				return -1;
			lexer->trans[state][index] = array_length(lexer->trans)
				- 1;
		}

		state = lexer->trans[state][index];
		prev_symbol = symbol;
	}
	lexer->final[state] = token->type;

	return 0;
}

static int
mflexer_trans_init(struct mflexer *lexer, const struct token *tokens)
{
	size_t tokens_length;
	int ret;

	lexer->trans = array_create(0, sizeof *lexer->trans);
	if (lexer->trans == NULL)
		return -1;

	lexer->final = array_create(0, sizeof *lexer->final);
	if (lexer->final == NULL)
		return -1;

	/*
	 * Argument `indice_count` should be the greatest encoding size amongst
	 * the encodings in `trans_indices`. Size of the alphabet will suffice.
	 */
	ret = mflexer_create_state(lexer, encoding_size(lexer->alphabet));
	if (ret < 0)
		return -1;

	ret = mflexer_create_state(lexer
		, encoding_size(lexer->trans_indices[lexer->alphabet[0]]));
	if (ret < 0)
		return -1;

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		ret = mflexer_insert_token(lexer, &tokens[i]);
		if (ret < 0) // cleanup..
			return -1;
	}

	return 0;
}

int mflexer_init(struct mflexer *lexer, FILE *file)
{
	struct token *tokens;
	int ret;

	tokens = array_create(0, sizeof *tokens);
	if (tokens == NULL)
		return -1;

	ret = read_tokens(file, &tokens);
	if (ret < 0)
		return -1;

	ret = mflexer_alphabet_init(lexer, tokens);
	if (ret < 0)
		return -1;

	ret = mflexer_trans_indices_init(lexer, tokens);
	if (ret < 0)
		return -1;

	ret = mflexer_trans_init(lexer, tokens);
	if (ret < 0)
		return -1;

	return 0;
}

void mflexer_destroy(struct mflexer *lexer)
{
	void **temp;

	while ((temp = array_pop(lexer->trans)) != NULL)
		array_destroy(*temp);
	array_destroy(lexer->trans);

	while ((temp = array_pop(lexer->trans_indices)) != NULL)
		encoding_destroy(*temp);
	array_destroy(lexer->trans_indices);

	encoding_destroy(lexer->alphabet);
}

int mflexer_dump(const struct mflexer *lexer, FILE *file)
{
	size_t ret, *meta;

	meta = array_create(2 + array_length(lexer->trans), sizeof *meta);
	if (meta == NULL)
		return -1;

	meta[0] = encoding_size(lexer->alphabet);
	meta[1] = array_length(lexer->trans);
	for (size_t i = 0; i < meta[1]; i++)
		meta[i + 2] = array_length(lexer->trans[i]);

	ret = fwrite(meta, sizeof *meta, array_length(meta), file);
	if (ret != array_length(meta))
		return -1;

	ret = fwrite(lexer->alphabet, sizeof *lexer->alphabet
		, MFLEXER_ALPHABET_SIZE, file);
	if (ret != MFLEXER_ALPHABET_SIZE)
		return -1;

	for (size_t i = 0; i < meta[0]; i++) {
		ret = fwrite(lexer->trans_indices[i]
			, sizeof *lexer->trans_indices[i], meta[0], file);
		if (ret != meta[0])
			return -1;
	}

	for (size_t i = 0; i < meta[1]; i++) {
		ret = fwrite(lexer->trans[i], sizeof *lexer->trans[i]
			, meta[i + 2], file);
		if (ret != meta[i + 2])
			return -1;
	}

	ret = fwrite(lexer->final, sizeof *lexer->final, meta[1], file);
	if (ret != meta[1])
		return -1;

	array_destroy(meta);

	return 0;
}

void mflexer_log(const struct mflexer *lexer)
{
	size_t temp;

	for (int i = 0; i < MFLEXER_ALPHABET_SIZE; i++)
		printf(isprint(i) ? "'%c': %hhu\n" : "%d: %hhu\n", i
			, lexer->alphabet[i]);
	printf("\n");

	temp = encoding_size(lexer->alphabet);
	for (int i = 0; i < temp; i++) {
		printf("%d: ", i);
		for (int j = 0; j < temp; j++)
			printf("%hhu ", lexer->trans_indices[i][j]);
		printf("\n");
	}
	printf("\n");

	temp = array_length(lexer->trans);
	for (size_t i = 0; i < temp; i++) {
		size_t temp;

		temp = array_length(lexer->trans[i]);

		printf("%zd: ", i);
		for (size_t j = 0; j < temp; j++)
			printf("%u ", lexer->trans[i][j]);
		printf("\n");
	}

	fflush(stdout);
}
