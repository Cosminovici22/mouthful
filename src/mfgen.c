#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "assert.h"
#include "encoding.h"
#include "common.h"

struct token {
	int type;
	char *value;
};

static int read_tokens(FILE *file, struct token **tokens)
{
	while (!feof(file)) {
		struct token token, *temp;
		int ret;

		ret = fscanf(file, " %9u : %ms ", &token.type, &token.value);
		if (ret != 2)
			return MFSTATUS_ERR_IO;

		for (size_t i = 0; token.value[i] != '\0'; i++) {
			if (token.value[i] != '\\')
				continue;

			token.value[i + 1] = token.value[i + 1] == 'f'
				? '\f'
				: token.value[i + 1] == 'n'
				? '\n'
				: token.value[i + 1] == 'r'
				? '\r'
				: token.value[i + 1] == 's'
				? ' '
				: token.value[i + 1] == 't'
				? '\t'
				: token.value[i + 1] == 'v'
				? '\v'
				: token.value[i + 1];

			memmove(token.value + i, token.value + i + 1
				, strlen(token.value + i + 1) + 1);
		}

		temp = array_push(*tokens, &token);
		if (temp == NULL)
			return MFSTATUS_ERR_MEM;
		*tokens = temp;
	}

	return MFSTATUS_SUCCESS;
}

static int
mflexer_alphabet_init(struct mflexer *lexer, const struct token *tokens)
{
	size_t tokens_length;

	lexer->alphabet = encoding_create(MFLEXER_ALPHABET_SIZE);
	if (lexer->alphabet == NULL)
		return MFSTATUS_ERR_MEM;

	tokens_length = array_length(tokens);
	encoding_add(lexer->alphabet, 0);
	for (size_t i = 0; i < tokens_length; i++)
		for (size_t j = 0; tokens[i].value[j] != '\0'; j++) {
			mfbyte_t byte;

			byte = tokens[i].value[j];
			encoding_add(lexer->alphabet, byte);
		}

	return MFSTATUS_SUCCESS;
}

static int
mflexer_trans_indices_init(struct mflexer *lexer, const struct token *tokens)
{
	size_t temp, tokens_length;

	temp = encoding_size(lexer->alphabet);

	lexer->trans_indices = array_create(temp, sizeof *lexer->trans_indices);
	if (lexer->trans_indices == NULL)
		return MFSTATUS_ERR_MEM;

	for (size_t i = 0; i < temp; i++) {
		lexer->trans_indices[i] = encoding_create(temp);
		if (lexer->trans_indices[i] == NULL)
			goto err;
	}

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		mfbyte_t prev_symbol;

		prev_symbol = lexer->alphabet[0];
		for (size_t j = 0; tokens[i].value[j] != '\0'; j++) {
			mfbyte_t byte, symbol;

			byte = tokens[i].value[j];
			symbol = lexer->alphabet[byte];
			encoding_add(lexer->trans_indices[prev_symbol], symbol);

			prev_symbol = symbol;
		}
	}

	return MFSTATUS_SUCCESS;

err:
	for (size_t i = 0; lexer->trans_indices[i] != NULL; i++)
		encoding_destroy(lexer->trans_indices[i]);
	array_destroy(lexer->trans_indices);

	return MFSTATUS_ERR_MEM;
}

static int
mflexer_create_state(struct mflexer *lexer, size_t indice_count)
{
	mfstate_t *indices;
	void *temp;
	int type;

	indices = array_create(indice_count, sizeof *indices);
	if (indices == NULL)
		return MFSTATUS_ERR_MEM;
	memset(indices, MFLEXER_ERROR_STATE
		, sizeof *indices * array_length(indices));

	temp = array_push(lexer->trans, &indices);
	if (temp == NULL)
		goto err_lexer_trans_alloc;
	lexer->trans = temp;

	type = MFLEXER_INVALID_TOK;
	temp = array_push(lexer->final, &type);
	if (temp == NULL)
		goto err_lexer_final_alloc;
	lexer->final = temp;

	return MFSTATUS_SUCCESS;

err_lexer_final_alloc:
	array_pop(lexer->trans);
err_lexer_trans_alloc:
	array_destroy(indices);

	return MFSTATUS_ERR_MEM;
}

static int
mflexer_insert_token(struct mflexer *lexer, const struct token *token)
{
	mfstate_t state;
	mfbyte_t prev_symbol;

	state = MFLEXER_START_STATE;
	prev_symbol = lexer->alphabet[0];
	for (size_t i = 0; token->value[i] != '\0'; i++) {
		mfbyte_t byte, symbol, index;

		byte = token->value[i];
		symbol = lexer->alphabet[byte];
		index = lexer->trans_indices[prev_symbol][symbol];

		if (lexer->trans[state][index] == 0) {
			int ret;

			ret = mflexer_create_state(lexer
				, encoding_size(lexer->trans_indices[symbol]));
			if (ret != MFSTATUS_SUCCESS)
				return ret;
			lexer->trans[state][index] = array_length(lexer->trans)
				- 1;
		}

		state = lexer->trans[state][index];
		prev_symbol = symbol;
	}
	lexer->final[state] = token->type;

	return MFSTATUS_SUCCESS;
}

static int
mflexer_trans_init(struct mflexer *lexer, const struct token *tokens)
{
	size_t tokens_length;
	void **temp;
	int ret;

	lexer->trans = array_create(0, sizeof *lexer->trans);
	if (lexer->trans == NULL)
		return MFSTATUS_ERR_MEM;

	lexer->final = array_create(0, sizeof *lexer->final);
	if (lexer->final == NULL)
		goto err_lexer_final_alloc;

	/*
	 * Argument `indice_count` should be the greatest encoding size amongst
	 * the encodings in `trans_indices`. Size of the alphabet will suffice.
	 */
	ret = mflexer_create_state(lexer, encoding_size(lexer->alphabet));
	if (ret != MFSTATUS_SUCCESS)
		goto err_lexer_trans_alloc;

	ret = mflexer_create_state(lexer
		, encoding_size(lexer->trans_indices[lexer->alphabet[0]]));
	if (ret != MFSTATUS_SUCCESS)
		goto err_lexer_trans_alloc;

	tokens_length = array_length(tokens);
	for (size_t i = 0; i < tokens_length; i++) {
		ret = mflexer_insert_token(lexer, &tokens[i]);
		if (ret != MFSTATUS_SUCCESS)
			goto err_lexer_trans_alloc;
	}

	return MFSTATUS_SUCCESS;

err_lexer_trans_alloc:
	array_destroy(lexer->final);

err_lexer_final_alloc:
	while ((temp = array_pop(lexer->trans)) != NULL)
		array_destroy(*temp);
	array_destroy(lexer->trans);

	return MFSTATUS_ERR_MEM;
}

static int mflexer_gen(struct mflexer *lexer, FILE *file)
{
	struct token *tokens, *token;
	int ret;

	tokens = array_create(0, sizeof *tokens);
	if (tokens == NULL)
		return MFSTATUS_ERR_MEM;

	ret = read_tokens(file, &tokens);
	if (ret != MFSTATUS_SUCCESS)
		goto out;

	ret = mflexer_alphabet_init(lexer, tokens);
	if (ret != MFSTATUS_SUCCESS)
		goto out;

	ret = mflexer_trans_indices_init(lexer, tokens);
	if (ret != MFSTATUS_SUCCESS)
		goto out;

	ret = mflexer_trans_init(lexer, tokens);

out:
	while ((token = array_pop(tokens)) != NULL)
		free(token->value);
	array_destroy(tokens);

	return ret;
}

static void mflexer_destroy(struct mflexer *lexer)
{
	void **temp;

	array_destroy(lexer->final);

	while ((temp = array_pop(lexer->trans)) != NULL)
		array_destroy(*temp);
	array_destroy(lexer->trans);

	while ((temp = array_pop(lexer->trans_indices)) != NULL)
		encoding_destroy(*temp);
	array_destroy(lexer->trans_indices);

	encoding_destroy(lexer->alphabet);
}

static int mflexer_dump(const struct mflexer *lexer, FILE *file)
{
	size_t ret, *meta;
	int rc;

	rc = MFSTATUS_SUCCESS;

	meta = array_create(2 + array_length(lexer->trans), sizeof *meta);
	if (meta == NULL)
		return MFSTATUS_ERR_MEM;

	meta[0] = encoding_size(lexer->alphabet);
	meta[1] = array_length(lexer->trans);
	for (size_t i = 0; i < meta[1]; i++)
		meta[i + 2] = array_length(lexer->trans[i]);

	ret = fwrite(meta, sizeof *meta, array_length(meta), file);
	if (ret != array_length(meta)) {
		rc = MFSTATUS_ERR_IO;
		goto out;
	}

	ret = fwrite(lexer->alphabet, sizeof *lexer->alphabet
		, MFLEXER_ALPHABET_SIZE, file);
	if (ret != MFLEXER_ALPHABET_SIZE) {
		rc = MFSTATUS_ERR_IO;
		goto out;
	}

	for (size_t i = 0; i < meta[0]; i++) {
		ret = fwrite(lexer->trans_indices[i]
			, sizeof *lexer->trans_indices[i], meta[0], file);
		if (ret != meta[0]) {
			rc = MFSTATUS_ERR_IO;
			goto out;
		}
	}

	for (size_t i = 0; i < meta[1]; i++) {
		ret = fwrite(lexer->trans[i], sizeof *lexer->trans[i]
			, meta[i + 2], file);
		if (ret != meta[i + 2]) {
			rc = MFSTATUS_ERR_IO;
			goto out;
		}
	}

	ret = fwrite(lexer->final, sizeof *lexer->final, meta[1], file);
	if (ret != meta[1]) {
		rc = MFSTATUS_ERR_IO;
		goto out;
	}

out:
	array_destroy(meta);

	return rc;
}

#ifdef DEBUG
static void mflexer_log(const struct mflexer *lexer)
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
#endif /* DEBUG */

int main(int argc, char *argv[])
{
	FILE *infile, *outfile;
	struct mflexer lexer;
	int ret;

	ASSERT(argc == 3, "Usage: mfgen <infile> <outfile>\n");

	infile = fopen(argv[1], "rt");
	ASSERT(infile != NULL, "%s: unable to open '%s' for reading\n"
		, STATUS_STRING(MFSTATUS_ERR_IO), argv[1]);

	outfile = fopen(argv[2], "wb");
	ASSERT(outfile != NULL, "%s: unable to open '%s' for writing\n"
		, STATUS_STRING(MFSTATUS_ERR_IO), argv[2]);

	ret = mflexer_gen(&lexer, infile);
	ASSERT(ret == MFSTATUS_SUCCESS, "%s: unable to generate lexer\n"
		, STATUS_STRING(ret));

	ret = mflexer_dump(&lexer, outfile);
	ASSERT(ret == MFSTATUS_SUCCESS , "%s: unable to dump lexer to '%s'\n"
		, STATUS_STRING(ret), argv[2]);

	mflexer_destroy(&lexer);
	fclose(infile);
	fclose(outfile);

	return EXIT_SUCCESS;
}
