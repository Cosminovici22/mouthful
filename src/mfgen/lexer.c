#include "lexer.h"

#include <stddef.h>
#include <string.h>

#include "array.h"
#include "encoding.h"
#include "token.h"

static int lexer_alphabet_init(struct lexer *lexer, struct token *tokens)
{
	size_t tokens_length;

	lexer->alphabet = encoding_create(128);
	if (lexer->alphabet == NULL)
		return -1;

	tokens_length = array_length(tokens);
	encoding_add(lexer->alphabet, 0);
	for (size_t i = 0; i < tokens_length; i++)
		for (size_t j = 0; tokens[i].value[j] != '\0'; j++) {
			byte_t byte;

			byte = tokens[i].value[j];
			encoding_add(lexer->alphabet, byte);
		}

	return 0;
}

static int lexer_trans_indices_init(struct lexer *lexer, struct token *tokens)
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
		byte_t prev_symbol;

		prev_symbol = lexer->alphabet[0];
		for (size_t j = 0; tokens[i].value[j] != '\0'; j++) {
			byte_t byte, symbol;

			byte = tokens[i].value[j];
			symbol = lexer->alphabet[byte];
			encoding_add(lexer->trans_indices[prev_symbol], symbol);

			prev_symbol = symbol;
		}
	}

	return 0;
}

static int lexer_create_state(struct lexer *lexer, size_t indice_count)
{
	state_t *indices;
	int type;

	indices = array_create(indice_count, sizeof *indices);
	if (indices == NULL)
		return -1;
	memset(indices, 0, sizeof *indices * array_length(indices));

	lexer->trans = array_push(lexer->trans, &indices);
	if (lexer->trans == NULL)
		return -1;

	type = -1;
	lexer->final = array_push(lexer->final, &type);
	if (lexer->final == NULL)
		return -1;

	return 0;
}

static int lexer_insert_token(struct lexer *lexer, struct token *token)
{
	state_t state;
	byte_t prev_symbol;

	state = lexer->start;
	prev_symbol = lexer->alphabet[0];
	for (size_t i = 0; token->value[i] != '\0'; i++) {
		byte_t byte, symbol, index;

		byte = token->value[i];
		symbol = lexer->alphabet[byte];
		index = lexer->trans_indices[prev_symbol][symbol];

		if (lexer->trans[state][index] == 0) {
			int ret;

			ret = lexer_create_state(lexer
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

static int lexer_trans_init(struct lexer *lexer, struct token *tokens)
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
	 * The transition table of states directly accessibe from more than one
	 * state is more difficult to obtain. In the case of the error state,
	 * argument `indice_count` should be the greatest encoding size amongst
	 * the encodings in `trans_indices`. For now, the size of the alphabet
	 * will suffice.
	 */
	ret = lexer_create_state(lexer, encoding_size(lexer->alphabet));
	if (ret < 0)
		return -1;

	ret = lexer_create_state(lexer
		, encoding_size(lexer->trans_indices[lexer->alphabet[0]]));
	if (ret < 0)
		return -1;
	lexer->start = ret;

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		ret = lexer_insert_token(lexer, &tokens[i]);
		if (ret < 0) // cleanup..
			return -1;
	}

	return 0;
}

int lexer_init(struct lexer *lexer, struct token *tokens)
{
	int ret;

	ret = lexer_alphabet_init(lexer, tokens);
	if (ret < 0)
		return -1;

	ret = lexer_trans_indices_init(lexer, tokens);
	if (ret < 0)
		return -1;

	ret = lexer_trans_init(lexer, tokens);
	if (ret < 0)
		return -1;

	return 0;
}

void lexer_destroy(struct lexer *lexer)
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

int lexer_accepts(struct lexer *lexer, byte_t *string)
{
	state_t state;
	byte_t prev_symbol;

	state = lexer->start;
	prev_symbol = lexer->alphabet[0];
	for (size_t i = 0; string[i] != '\0'; i++) {
		byte_t byte, symbol, index;

		byte = string[i];
		symbol = lexer->alphabet[byte];
		index = lexer->trans_indices[prev_symbol][symbol];

		state = lexer->trans[state][index];
		prev_symbol = symbol;
	}

	return lexer->final[state];
}
