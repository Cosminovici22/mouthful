#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "encoding.h"

typedef unsigned int state_t;

struct lexer {
	byte_t *alphabet;
	byte_t **trans_indices;
	state_t **trans;
	state_t start;
	int *final;
};

int lexer_init(struct lexer *lexer, struct token *tokens);
void lexer_destroy(struct lexer *lexer);
int lexer_accepts(struct lexer *lexer, byte_t *string);

#endif
