#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "dfa.h"
#include "token.h"

int read_tokens(FILE *fin, struct array *tokens)
{
	while (!feof(fin)) {
		struct token *token;
		int ret;

		token = malloc(sizeof *token);
		if (token == NULL)
			return 1;
		array_push(tokens, token);

		ret = fscanf(fin, " %9u : %ms ", &token->type, &token->value);
		if (ret != 2)
			return 1;
	}

	return 0;
}

int write_dfa(FILE *fout, struct dfa *dfa)
{
	for (int i = 33; i < 127; i++)
		printf("%c: %hhu\n", i, dfa->alphabet[i]);

	for (int i = 0; i < dfa->alphabet_size + 1; i++) {
		printf("%d: ", i);
		for (int j = 0; j < dfa->alphabet_size; j++)
			printf("%d ", dfa->trans_indices[i][j]);
		printf("\n");
	}

	for (int i = 0; i < dfa->state_trans.size; i++) {
		printf("%d: ", i);
		for (int j = 0; j < *(uint8_t *) dfa->state_trans_sizes.elems[i]; j++)
			printf("%d ", ((uint32_t *) dfa->state_trans.elems[i])[j]);
		printf("\n");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fin, *fout;
	struct array tokens;
	struct dfa dfa;
	int ret;

	fin = fopen(argv[1], "rt");
	if (fin == NULL)
		return 1;

	// fout = fopen(argv[2], "rt");
	// if (fout == NULL)
	// 	return 1;

	array_init(&tokens);

	ret = read_tokens(fin, &tokens);
	if (ret != 0)
		return 1;

	ret = dfa_init(&dfa, &tokens);
	if (ret != 0)
		return 1;

	array_destroy(&tokens); // each token and token->value must be freed

	ret = write_dfa(fout, &dfa);
	if (ret != 0)
		return 1;

	dfa_destroy(&dfa);
	fclose(fin);
	// fclose(fout);

	return 0;
}
