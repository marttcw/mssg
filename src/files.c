#include "files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>

enum {
	FDP_DIR = 4,
	FDP_FIL = 8
};

static void
files__file_cleanup(void *data)
{
	(void) data;
}

struct files
files_create(const char *start_dir)
{
	struct files files = {
		.list = {
			.list = NULL,
			.length = 0,
			.allocated = 0,
			.ALLOC_CHUNK = 16,
			.type_size = sizeof(struct file),
			.is_pointer = false,
			.cleanup = files__file_cleanup
		},
		.start_dir = { 0 },
		.allowed = {
			.list = NULL,
			.length = 0,
			.allocated = 0,
			.ALLOC_CHUNK = 16,
			.type_size = sizeof(struct file_allowed),
			.is_pointer = false,
			.cleanup = NULL
		},
	};

	strcpy(files.start_dir, start_dir);

	return files;
}

void
files_destroy(struct files *files)
{
	generic_list_destroy(&files->list);
}

struct file *
files_next(struct files *files)
{
	return NULL;
}

static void
files__add_file(struct files *files,
		const char *fullpath,
		const char *filename,
		const uint32_t type)
{
	struct file *file = generic_list_add(&files->list);
	switch (type)
	{
	case FDP_DIR:
		file->type = FTYPE_DIR;
		break;
	case FDP_FIL:
		file->type = FTYPE_GEN;
		break;
	default:
		file->type = FTYPE_COPY;
	}
	sprintf(file->path_ful, fullpath);
	sprintf(file->path_rel, filename);
	sprintf(file->path_gen, " ");
	file->parsed = false;
}

static char *
files__get_file_ext(const char *filename)
{
	char *dot = strrchr(filename, '.');
	return (!dot || dot == filename) ? "" : (dot + 1);
}

static void
files__traverse_dir(struct files *files,
		const char *dirpath)
{
	struct dirent *dp;
	DIR *dir = opendir(dirpath);

	if (!dir)
	{
		return;
	}

	while ((dp = readdir(dir)) != NULL)
	{
		if ((dp->d_name[0] == '.'))
		{
			continue;
		}

		const char *ext = files__get_file_ext(dp->d_name);
		char recdirpath[512] = { 0 };
		sprintf(recdirpath, "%s/%s", dirpath, dp->d_name);

		if ((!strcmp(dp->d_name, "config")) ||
			((!strcmp(ext, "html") || !strcmp(ext, "css")) &&
			 	files_allowed(files, dp->d_name)))
		{
			//printf("%s %s %d\n", recdirpath, dp->d_name, dp->d_type);
			files__add_file(files, recdirpath, dp->d_name, dp->d_type);
		}
		else if (dp->d_type == FDP_DIR)
		{
			files__traverse_dir(files, recdirpath);
		}
	}

	closedir(dir);
}

void
files_traverse(struct files *files)
{
	files__traverse_dir(files, files->start_dir);
}

void
files_set_parsed(struct files *files,
		const char *path,
		void *parser)
{
	const uint32_t length = files->list.length;
	for (uint32_t i = 0; i < length; ++i)
	{
		struct file *file = generic_list_get(&files->list, i);
		//printf("compare: %s vs %s\n", file->path_ful, path); // TEMP
		if (!strcmp(file->path_ful, path))
		{
			file->parsed = true;
			file->parser = parser;
			break;
		}
	}
}

void *
files_get_parser(struct files *files,
		const char *path)
{
	const uint32_t length = files->list.length;
	for (uint32_t i = 0; i < length; ++i)
	{
		struct file *file = generic_list_get(&files->list, i);
		if (!strcmp(file->path_ful, path))
		{
			return file->parser;
		}
	}

	return NULL;
}

bool
files_allowed(struct files *files,
		const char *filename)
{
	const uint32_t length = files->allowed.length;
	for (uint32_t i = 0; i < length; ++i)
	{
		struct file_allowed *allowed = generic_list_get(
				&files->allowed, i);

		if (!strcmp(allowed->filename, filename))
		{
			return true;
		}
	}

	return false;
}

void
files_allowed_add(struct files *files,
		const char *filename)
{
	struct file_allowed *allowed = generic_list_add(&files->allowed);
	strcpy(allowed->filename, filename);
}

