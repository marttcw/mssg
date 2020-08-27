#include <stdio.h>

#include "parser.h"
#include "minify.h"

int
main(int argc, char **argv)
{
	int retval = 0;
	(void) argc;
	(void) argv;

	struct parser parser = { 0 };
	parser_create(&parser, "site/index.html", "site");
	if (parser.error != PARSER_ERROR_NONE)
	{
		fprintf(stderr, "Error code: %d | Message: %s\n",
				parser.error, parser_error_message(&parser));
		retval = 1;
		goto cleanup;
	}
	parser_print(&parser);
	FILE *tmp_file = tmpfile();
	parser_generate(&parser, tmp_file);
	minify(stdout, tmp_file);
	fclose(tmp_file);

cleanup:
	parser_destroy(&parser);
	parser_deinit();
	templates_deinit();

	return retval;
}

