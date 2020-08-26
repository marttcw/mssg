#include <stdio.h>

#include "parser.h"
#include "minify.h"

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
	//parser_print(&parser);
	FILE *tmp_file = tmpfile();
	parser_generate(&parser, tmp_file);
	minify(stdout, tmp_file);
	fclose(tmp_file);

	parser_destroy(&parser);
	parser_deinit();
	templates_deinit();

	return 0;
}

