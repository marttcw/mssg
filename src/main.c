#include <stdio.h>

#define GENERIC_LIST_IMPLEMENTATION_H
#include "generic_list.h"

#include "parser.h"
#include "minify.h"
#include "files.h"

int
main(int argc, char **argv)
{
	int retval = 0;
	(void) argc;
	(void) argv;

	struct files files = files_create(".");
	files_allowed_add(&files, "index.html");

	files_traverse(&files);

	struct parser parser = { 0 };
	parser_create(&parser, "./src/index.html", "./src", &files);
	if (parser.error != PARSER_ERROR_NONE)
	{
		fprintf(stderr, "Error code: %d | Message: %s\n",
				parser.error, parser_error_message(&parser));
		retval = 1;
		goto cleanup;
	}
	//parser_print(&parser);
	FILE *tmp_file = tmpfile();
	parser_generate(&parser, tmp_file);
	minify(stdout, tmp_file);
	fclose(tmp_file);

cleanup:
	parser_destroy(&parser);
	parser_deinit();
	templates_deinit();
	files_destroy(&files);

	return retval;
}

