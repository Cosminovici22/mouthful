#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "lexer.h"
#include "encoding.h"
#include "token.h"

int read_tokens(FILE *fin, struct token **tokens)
{
	while (!feof(fin)) {
		struct token token, *aux;
		int ret;

		ret = fscanf(fin, " %9u : %ms ", &token.type, &token.value);
		if (ret != 2) // might not work
			return -1;

		aux = array_push(*tokens, &token);
		if (aux == NULL)
			return -1;
		*tokens = aux;
	}

	return 0;
}

int write_lexer(FILE *fout, struct lexer *lexer)
{
	for (int i = 33; i < 127; i++)
		printf("%c: %hhu\n", i, lexer->alphabet[i]);

	for (int i = 0; i < array_length(lexer->trans_indices); i++) {
		printf("%d: ", i);
		for (int j = 0; j < encoding_size(lexer->alphabet); j++)
			printf("%d ", lexer->trans_indices[i][j]);
		printf("\n");
	}

	for (int i = 0; i < array_length(lexer->trans); i++) {
		printf("%d: ", i);
		for (int j = 0; j < array_length(lexer->trans[i]); j++)
			printf("%d ", lexer->trans[i][j]);
		printf("\n");
	}

	// printf("%d\n", lexer_accepts(lexer, "abracadabra"));
	// printf("%d\n", lexer_accepts(lexer, "sdsada"));
	// printf("%d\n", lexer_accepts(lexer, "lasjflasjf"));
	// printf("%d\n", lexer_accepts(lexer, "lexerfafagg"));
	// printf("%d\n", lexer_accepts(lexer, "btbtb"));
	// printf("%d\n", lexer_accepts(lexer, "lexerfafagga"));
	// printf("%d\n", lexer_accepts(lexer, "u"));
	// printf("%d\n", lexer_accepts(lexer, "{}udsa39879(_-=++)`~"));
	// printf("%d\n", lexer_accepts(lexer, "ab"));

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fin, *fout;
	struct token *tokens;
	struct lexer lexer;
	int ret;

	fin = fopen(argv[1], "rt");
	if (fin == NULL)
		return -1;

	// fout = fopen(argv[2], "wb");
	// if (fout == NULL)
	// 	return -1;

	tokens = array_create(0, sizeof *tokens);

	ret = read_tokens(fin, &tokens);
	if (ret < 0)
		return -1;

	ret = lexer_init(&lexer, tokens);
	if (ret < 0)
		return -1;

	while (array_length(tokens) != 0) {
		struct token *token;

		token = array_pop(tokens);
		free(token->value);
	}
	array_destroy(tokens);

	ret = write_lexer(fout, &lexer);
	if (ret < 0)
		return -1;

	lexer_destroy(&lexer);
	fclose(fin);
	// fclose(fout);

	return 0;
}
