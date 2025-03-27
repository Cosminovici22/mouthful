#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/mftok.h"
#include "assert.h"
#include "common.h"

static long file_size(FILE *file)
{
	long old_pos, new_pos;
	int ret;

	old_pos = ftell(file);
	if (old_pos < 0)
		return old_pos;

	ret = fseek(file, 0, SEEK_END);
	if (ret < 0)
		return ret;

	new_pos = ftell(file);
	if (new_pos < 0)
		return new_pos;

	ret = fseek(file, old_pos, SEEK_SET);
	if (ret < 0)
		return ret;

	return new_pos - old_pos;
}

int mflexer_init(struct mflexer *lexer, FILE *file)
{
	size_t ret, state_count;
	mfbyte_t alphabet_size, *sizes;
	long lexer_size;
	int rc;

	ret = fread(&alphabet_size, sizeof alphabet_size, 1, file);
	if (ret != 1)
		return MFSTATUS_ERR_IO;

	ret = fread(&state_count, sizeof state_count, 1, file);
	if (ret != 1)
		return MFSTATUS_ERR_IO;

	sizes = malloc(state_count * sizeof *sizes);
	if (sizes == NULL)
		return MFSTATUS_ERR_MEM;

	ret = fread(sizes, sizeof *sizes, state_count, file);
	if (ret != state_count) {
		rc = MFSTATUS_ERR_IO;
		goto err_alphabet_alloc;
	}

	lexer_size = file_size(file);
	if (lexer_size < 0) {
		rc = MFSTATUS_ERR_IO;
		goto err_alphabet_alloc;
	}

	lexer->alphabet = malloc(lexer_size);
	if (lexer->alphabet == NULL) {
		rc = MFSTATUS_ERR_MEM;
		goto err_alphabet_alloc;
	}

	ret = fread(lexer->alphabet, 1, lexer_size, file);
	if (ret != lexer_size) {
		rc = MFSTATUS_ERR_IO;
		goto err_trans_indices_alloc;
	}

	lexer->trans_indices = malloc((alphabet_size + 1)
		* sizeof *lexer->trans_indices);
	if (lexer->trans_indices == NULL) {
		rc = MFSTATUS_ERR_MEM;
		goto err_trans_indices_alloc;
	}

	lexer->trans_indices[0] = (void *) lexer->alphabet
		+ MFLEXER_ALPHABET_SIZE * sizeof *lexer->alphabet;
	for (size_t i = 1; i < alphabet_size + 1; i++)
		lexer->trans_indices[i] = (void *) lexer->trans_indices[i - 1]
			+ (alphabet_size + 1) * sizeof *lexer->alphabet;

	lexer->trans = malloc(state_count * sizeof *lexer->trans);
	if (lexer->trans == NULL) {
		rc = MFSTATUS_ERR_MEM;
		goto err_trans_alloc;
	}

	lexer->trans[0] = (void *) lexer->trans_indices[alphabet_size]
		+ (alphabet_size + 1) * sizeof *lexer->alphabet;
	for (size_t i = 1; i < state_count; i++)
		lexer->trans[i] = (void *) lexer->trans[i - 1] + (sizes[i - 1]
			+ 1) * sizeof **lexer->trans;

	lexer->final = (void *) lexer->trans[state_count - 1]
		+ (sizes[state_count - 1] + 1) * sizeof **lexer->trans;

	free(sizes);

	return MFSTATUS_SUCCESS;

err_trans_alloc:
	free(lexer->trans_indices);

err_trans_indices_alloc:
	free(lexer->alphabet);

err_alphabet_alloc:
	free(sizes);

	return rc;
}

void mflexer_destroy(struct mflexer *lexer)
{
	free(lexer->trans_indices);
	free(lexer->trans);
	free(lexer->alphabet);
}

int mflexer_tokenize_stream(const struct mflexer *lexer, FILE *file)
{
	mfstate_t state;
	mfbyte_t prev_symbol;
	int type, ret;

	state = MFLEXER_START_STATE;
	prev_symbol = lexer->alphabet[0];
	while (state != MFLEXER_ERROR_STATE) {
		mfbyte_t byte, symbol, index;

		type = lexer->final[state];

		ret = getc(file);
		if (ret < 0)
			return feof(file) ? type : MFSTATUS_ERR_IO;

		byte = ret;
		symbol = lexer->alphabet[byte];
		index = lexer->trans_indices[prev_symbol][symbol];

		state = lexer->trans[state][index];
		prev_symbol = symbol;
	}

	ret = ungetc(ret, file);
	if (ret < 0)
		return MFSTATUS_ERR_IO;

	return type;
}

int mflexer_tokenize_string(const struct mflexer *lexer, const char *string
	, size_t *offset)
{
	mfstate_t state;
	mfbyte_t prev_symbol;
	int type;

	state = MFLEXER_START_STATE;
	prev_symbol = lexer->alphabet[0];
	while (state != MFLEXER_ERROR_STATE) {
		mfbyte_t byte, symbol, index;

		type = lexer->final[state];

		if (string[*offset] == '\0')
			return type;

		byte = string[(*offset)++];
		symbol = lexer->alphabet[byte];
		index = lexer->trans_indices[prev_symbol][symbol];

		state = lexer->trans[state][index];
		prev_symbol = symbol;
	}
	(*offset)--;

	return type;
}

int main(int argc, char *argv[])
{
	FILE *lexer_dump, *file;
	struct mflexer lexer;
	int ret;

	ASSERT(argc == 3, "Usage: mftok <lexer_dump> <file>\n");

	lexer_dump = fopen(argv[1], "rb");
	ASSERT(lexer_dump != NULL, "%s: unable to open '%s' for reading\n"
		, STATUS_STRING(MFSTATUS_ERR_IO), argv[1]);

	file = fopen(argv[2], "rt");
	ASSERT(file != NULL, "%s: unable to open '%s' for reading\n"
		, STATUS_STRING(MFSTATUS_ERR_IO), argv[2]);

	ret = mflexer_init(&lexer, lexer_dump);
	ASSERT(ret == MFSTATUS_SUCCESS, "%s: unable to initialize lexer from "
		"'%s'\n", STATUS_STRING(ret), argv[1]);

	fclose(lexer_dump);

	while (!feof(file)) {
		ret = mflexer_tokenize_stream(&lexer, file);
		ASSERT(ret != MFSTATUS_ERR_TOK, "%s: invalid token encountered"
			"\n", STATUS_STRING(ret));
		ASSERT(ret != MFSTATUS_ERR_IO, "%s: unable to read from '%s'\n"
			, STATUS_STRING(ret), argv[2]);

		ret = printf("%d\n", ret);
		ASSERT(ret >= 0, "%s: unable to write to standard output\n"
			, STATUS_STRING(MFSTATUS_ERR_IO));
	}

	mflexer_destroy(&lexer);
	fclose(file);

	return 0;
}
