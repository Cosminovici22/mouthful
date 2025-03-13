#ifndef MFLEXER_GEN_H
#define MFLEXER_GEN_H

#include <stdio.h>

#include "../include/mflexer.h"

int mflexer_init(struct mflexer *lexer, FILE *file);
void mflexer_destroy(struct mflexer *lexer);
int mflexer_dump(const struct mflexer *lexer, FILE *file);

#ifdef DEBUG
void mflexer_log(const struct mflexer *lexer);
#endif

#endif
