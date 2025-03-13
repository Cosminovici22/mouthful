#include <stdio.h>

#include "../include/mflexer.h"
#include "mflexer_gen.h"

int main(int argc, char *argv[])
{
	FILE *infile, *outfile;
	struct mflexer lexer;
	int ret;

	if (argc != 3) {
		fprintf(stderr, "usage: mfgen <infile> <outfile>\n");
		return -1;
	}

	infile = fopen(argv[1], "rt");
	if (infile == NULL)
		return -1;

	outfile = fopen(argv[2], "wb");
	if (outfile == NULL)
		return -1;

	ret = mflexer_init(&lexer, infile);
	if (ret < 0)
		return -1;

	ret = mflexer_dump(&lexer, outfile);
	if (ret < 0)
		return -1;

	mflexer_destroy(&lexer);
	fclose(infile);
	fclose(outfile);

	return 0;
}
