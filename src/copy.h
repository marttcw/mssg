#ifndef COPY_H
#define COPY_H

#include <stdint.h>
#include <stdbool.h>

enum copy_ftype {
	COPY_FTYPE_FILENAME = 0,
	COPY_FTYPE_FULLPATH,

	COPY_FTYPE_MAX
};

void copy_init(void);
void copy_deinit(void);
void copy_ignore(const char *ignore_path, const enum copy_ftype type);
bool copy_ignore_this(const char *path, const enum copy_ftype type);

#endif // COPY_H

