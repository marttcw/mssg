#include "files.h"
#include "states.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define ALLOC_FILES (2560)
#define PATH_SIZE (1024)

char *
make_rel(const char *base_path, const char *full_path)
{
	int i;
	char *rel_path = (char *) calloc(PATH_SIZE, sizeof(char));

	for (i = 0; base_path[i] == full_path[i]; ++i) {
	}

	strncpy(rel_path, full_path+(i), strlen(full_path)-i);
	return rel_path;
}

files *
files_init(void)
{
	files *f = (files *) malloc(sizeof(files));

	f->fil = (file_info *) malloc(ALLOC_FILES * sizeof(file_info));
	for (int i=0; i < ALLOC_FILES; ++i) {
		f->fil[i].type = -1;
		f->fil[i].path_full = (char *) calloc(PATH_SIZE, sizeof(char));
		f->fil[i].path_relative = (char *) calloc(PATH_SIZE, sizeof(char));
	}
	f->fii = 0;

	return f;
}

int
files_destroy(files *f)
{
	for (int i=0; i < ALLOC_FILES; ++i) {
		free(f->fil[i].path_full);
		free(f->fil[i].path_relative);
	}
	free(f->fil);

	return 0;
}

int
files_read(const char *filepath)
{
	state s;

	state_init(&s);
	state_set_level_file(&s, filepath);
	state_generate(&s);
	state_destroy(&s);

	return 0;
}

int
files_directory(files *f, const char *base_dirpath, const char *dirpath, int type)
{
	struct dirent *entry;
	DIR *dp = NULL;
	char *path_full = (char *) calloc(PATH_SIZE, sizeof(char));

	if (path_full == NULL) {
		perror("calloc");
		return -2;
	}

	if ((dp = opendir(dirpath)) == NULL) {
		perror("opendir");
		return -1;
	}

	while ((entry = readdir(dp))) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
			sprintf(path_full, "%s/%s", dirpath, entry->d_name);

			// 4 == Directory
			if (entry->d_type == 4) {
				// If in base directory
				if (type == -1) {
					if (!strcmp(entry->d_name, "src")) {
						files_directory(f, base_dirpath, path_full, 0);
					}
				} else {
					files_directory(f, base_dirpath, path_full, type);
				}
			} else if (entry->d_type != 4) {
				if (!strcmp(entry->d_name, "conf.toml")) {
					f->fil[f->fii].type = 1;
				} else if (type != -1) {
					f->fil[f->fii].type = type;
				}

				if (f->fil[f->fii].type != -1) {
					strcpy(f->fil[f->fii].path_full, path_full);
					strcpy(f->fil[f->fii].path_relative, make_rel(base_dirpath, path_full));
					++f->fii;
				}
			}
		}
	}

	if (path_full != NULL) {
		free(path_full);
	}
	closedir(dp);
	return 0;
}

int
files_traverse(files *f, const char *startpath)
{
	files_directory(f, startpath, startpath, -1);

	for (unsigned int i=0; i < f->fii; ++i) {
		printf("%d: %s\n", f->fil[i].type, f->fil[i].path_relative);
	}

	return 0;
}

