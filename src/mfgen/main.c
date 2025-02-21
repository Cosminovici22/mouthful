#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "dfa.h"
#include "token.h"

int read_tokens(FILE *fin, struct token **tokens)
{
	while (!feof(fin)) {
		struct token token,*aux;
		int ret;

		ret = fscanf(fin, " %9d : %ms ", &token.type, &token.value);
		if (ret != 2) // might not work
			return 1;

		aux = array_push(*tokens, &token);
		if (aux == NULL)
			return 1;
		*tokens = aux;
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

	for (int i = 0; i < array_length(dfa->state_trans); i++) {
		printf("%d: ", i);
		for (int j = 0; j < array_length(dfa->state_trans[i]); j++)
			printf("%d ", dfa->state_trans[i][j]);
		printf("\n");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fin, *fout;
	struct token *tokens;
	struct dfa dfa;
	int ret;

	fin = fopen(argv[1], "rt");
	if (fin == NULL)
		return 1;

	// fout = fopen(argv[2], "rt");
	// if (fout == NULL)
	// 	return 1;

	tokens = array_create(sizeof *tokens);

	ret = read_tokens(fin, &tokens);
	if (ret != 0)
		return 1;

	ret = dfa_init(&dfa, tokens);
	if (ret != 0)
		return 1;

	array_destroy(tokens); // each token->value must be freed

	ret = write_dfa(fout, &dfa);
	if (ret != 0)
		return 1;

	dfa_destroy(&dfa);
	fclose(fin);
	// fclose(fout);

	return 0;
}
