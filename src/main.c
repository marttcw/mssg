#include <stdio.h>

#include "parser.h"

int
main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	struct parser parser = parser_create("index.html");
	if (parser.error != PARSER_ERROR_NONE)
	{
		fprintf(stderr, "Error code: %d | Message: %s\n",
				parser.error, parser_error_message(&parser));
	}
	parser_print(&parser);
	parser_destroy(&parser);

	return 0;
}

