#ifndef FILES_H
#define FILES_H

#include <stdint.h>
#include <stdbool.h>

#include "generic_list.h"

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
};

struct file_allowed {
	char		filename[256];
};

struct files {
	struct generic_list	list;
	char			start_dir[256];
	char			base_src_dir[256];
	char			base_dst_dir[256];
	struct generic_list	allowed;
};

struct files files_create(const char *start_dir);
void files_destroy(struct files *files);
void files_traverse(struct files *files);
struct file *files_next(struct files *files);
void files_set_parsed(struct files *files,
		const char *path,
		void *parser);
void *files_get_parser(struct files *files,
		const char *path);
bool files_allowed(struct files *files,
		const char *filename);
void files_allowed_add(struct files *files,
		const char *filename);

#endif // FILES_H

