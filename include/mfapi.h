#ifndef MFAPI_H
#define MFAPI_H

#include <stdio.h>

#include "mflexer.h"

#define MFAPI_ERROR -2

int mflexer_init(struct mflexer *lexer, FILE *file);
void mflexer_destroy(struct mflexer *lexer);
int mflexer_tokenize(const struct mflexer *lexer, FILE *file);

#endif
