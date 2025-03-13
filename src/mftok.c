#include <stdio.h>

#include "../include/mfapi.h"
#include "../include/mflexer.h"

int main(int argc, char *argv[])
{
	FILE *lexer_dump, *file;
	struct mflexer lexer;
	int ret;

	if (argc != 3) {
		fprintf(stderr, "usage: mftok <lexer_dump> <file>\n");
		return -1;
	}

	lexer_dump = fopen(argv[1], "rb");
	if (lexer_dump == NULL)
		return -1;

	file = fopen(argv[2], "rt");
	if (file == NULL)
		return -1;

	ret = mflexer_init(&lexer, lexer_dump);
	if (ret < 0)
		return -1;

	fclose(lexer_dump);

	while ((ret = mflexer_tokenize(&lexer, file)) != -1) {
		if (ret == MFAPI_ERROR)
			return -1;

		ret = printf("%d\n", ret);
		if (ret < 0)
			return -1;
	}

	mflexer_destroy(&lexer);
	fclose(file);

	return 0;
}
