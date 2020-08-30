#include <stdio.h>

#define GENERIC_LIST_IMPLEMENTATION_H
#include "generic_list.h"

#include "parser.h"
#include "minify.h"
#include "files.h"

#define M_MKDIR_IMPLEMENTATION_H
#include "m_mkdir.h"

#include <string.h>

int
main(int argc, char **argv)
{
	int retval = 0;
	(void) argc;
	(void) argv;

	struct files files = files_create(".", "", "");
	files_allowed_add(&files, "index.html");

	files_traverse(&files);

#if 0
	struct file *config_file = files_get_config(&files);
	if (config_file != NULL)
	{
		printf("file: %d %s %s %s\n",
				config_file->type, config_file->path_ful,
				config_file->path_rel,
				config_file->path_gen);
	}
#endif

	// Go through the files
	for (struct file *file = NULL;
			(file = files_next(&files)) != NULL;
		)
	{
		if (!strcmp(file->path_rel, "config"))
		{	// Skip over configuration file
			continue;
		}

#if 0
		printf("file: %d %s %s %s\n",
				file->type, file->path_ful,
				file->path_rel, file->path_gen);
#endif
		struct parser parser = { 0 };
		parser_create(&parser, file->path_ful, files.base_src_dir,
				&files);
		if (parser.error != PARSER_ERROR_NONE)
		{
			fprintf(stderr, "Error code: %d | Message: %s\n",
					parser.error, parser_error_message(&parser));
			retval = 1;
			parser_destroy(&parser);
			goto cleanup;
		}
#if 0
		parser_print(&parser);
#endif
		FILE *tmp_file = tmpfile();
		parser_generate(&parser, tmp_file, files.start_dir,
				file->path_gen, files.base_dst_dir);
		printf("Destination file: %s\n", file->path_gen);
		m_mkdir(file->path_gen, 0777, false);
		FILE *dst_file = fopen(file->path_gen, "w");
		if (dst_file != NULL)
		{
			minify(dst_file, tmp_file);
			fclose(dst_file);
		}
		else
		{
			fprintf(stderr, "ERROR: Cannot open destination file"
					" for writing!\n");
		}
		fclose(tmp_file);
		parser_destroy(&parser);
	}

cleanup:
	parser_deinit();
	templates_deinit();
	files_destroy(&files);

	return retval;
}

