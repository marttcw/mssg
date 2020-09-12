#include <stdio.h>
#include <string.h>

#define GENERIC_LIST_IMPLEMENTATION_H
#include "generic_list.h"
#define HASHMAP_IMPLEMENTATION_H
#include "hashmap.h"
#define M_MKDIR_IMPLEMENTATION_H
#include "m_mkdir.h"

#include "parser.h"
#include "minify.h"
#include "files.h"
#include "config.h"
#include "copy.h"
#include "vars.h"

int
main(int argc, char **argv)
{
	int retval = 0;
	(void) argc;
	(void) argv;

	templates_init();
	copy_init();

	struct config config = { 0 };
	config_create(&config);

	struct files files = files_create(".", "", "");
	//files_allowed_add(&files, "index.html");

	files_traverse(&files);
	//files_print(&files);

	struct file *config_file = files_get_config(&files);
	if (config_file != NULL)
	{
#if 0
		printf("file: %d %s %s %s\n",
				config_file->type, config_file->path_ful,
				config_file->path_rel,
				config_file->path_gen);
#endif
		config_parser(&config, config_file->path_ful);
		//config_print(&config);
		config_template(&config, files.start_dir, files.base_src_dir);

		for (struct config_line *line = NULL;
				(line = config_iter_type(&config,
						TEMPLATE_SET_DIR)) != NULL;
		    )
		{
			const char *name = line->argv[0];
			const char *traverse = line->argv[1];

			uint32_t total = 0;
			char **paths = files_get_under_dirs(
					&files, traverse, &total);
			putchar('\n');
			for (uint32_t i = 0; i < total; ++i)
			{
				printf("path: %s\n", paths[i]);
			}
			vars_set(name, (const char **) paths, total);
			printf("set: '%s'\n", name);
			putchar('\n');
		}
	}
	else
	{
		printf("Config file not found!\n");
	}

	// Go through the files
	for (struct file *file = NULL;
			(file = files_next(&files)) != NULL;
		)
	{
		if (!strcmp(file->path_rel, "config"))
		{	// Skip over configuration file
			continue;
		}
		else if (copy_ignore_this(file->path_rel, COPY_FTYPE_FILENAME))
		{
			printf("\nIgnored: %s\n", file->path_rel);
			continue;
		}

		printf("\nMain parsing file: %s\n", file->path_rel);

#if 0
		printf("file: %d %s %s %s\n",
				file->type, file->path_ful,
				file->path_rel, file->path_gen);
#endif
		struct parser parser = { 0 };
		parser_create(&parser, file->path_ful, files.base_src_dir,
				&files, false);
		if (parser.error != PARSER_ERROR_NONE)
		{
			fprintf(stderr, "Error code: %d | Message: %s\n",
					parser.error, parser_error_message(&parser));
			retval = 1;
			parser_destroy_nofree(&parser);
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
			minify(dst_file, tmp_file, file->ext);
			fclose(dst_file);
		}
		else
		{
			fprintf(stderr, "ERROR: Cannot open destination file"
					" for writing!\n");
		}
		fclose(tmp_file);
		parser_destroy_nofree(&parser);
		file->ignore_in_destroy = true;
	}

cleanup:
	parser_deinit();
	copy_deinit();
	templates_deinit();
	files_destroy(&files);
	config_destroy(&config);

	return retval;
}

