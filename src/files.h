#ifndef FILES_H
#define FILES_H

typedef struct {
	int type;
	char *path_full;
	char *path_relative;
	char *make_path;
} file_info;

typedef struct {
	file_info *fil;		// File info list
	unsigned int fii;	// Current index
} files;

files *files_new(void);
int files_destroy(files *f);

int files_build(files *f, const char *startpath);
int init_site(const char *name);

#endif /* FILES_H */
