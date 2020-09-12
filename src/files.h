#ifndef FILES_H
#define FILES_H

#include <stdint.h>
#include <stdbool.h>

#include "generic_list.h"
#include "hashmap.h"

enum ftype {
	FTYPE_GEN = 0,
	FTYPE_DIR,
	FTYPE_COPY,

	FTYPE_MAX
};

struct file {
	enum ftype	type;
	char		path_ful[256];
	char		path_rel[256];
	char		path_gen[256];
	bool		parsed;
	void		*parser;
	char		ext[64];
	bool		ignore_in_destroy;
};

struct file_allowed {
	char a;
};

struct files {
	struct generic_list	list;
	char			start_dir[256];
	char			base_src_dir[256];
	char			base_dst_dir[256];
	struct hashmap		allowed;
	uint32_t		next_index;
	bool			ended;
};

struct files files_create(const char *start_dir,
		const char *src_dir,
		const char *dst_dir);
void files_destroy(struct files *files);
void files_traverse(struct files *files);
struct file *files_get(struct files *files,
		const char *filepath);
struct file *files_next(struct files *files);
struct file *files_get_config(struct files *files);
void files_set_parsed(struct files *files,
		const char *path,
		void *parser);
void *files_get_parser(struct files *files,
		const char *path);
bool files_allowed(struct files *files,
		const char *filename);
void files_allowed_add(struct files *files,
		const char *filename);
void files_print(struct files *files);
char **files_get_under_dirs(struct files *files,
		const char *startfix,
		uint32_t *total);

#endif // FILES_H

