#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 4096

static char current_path[PATH_MAX] = { '\0' };
static FILE *current_fp = NULL;

void
log_set_file(const char *filepath)
{
	current_fp = fopen(filepath, "w");
	strcpy(current_path, filepath);
}

void
log_deinit(void)
{
	if (current_fp != NULL)
	{
		fclose(current_fp);
	}
}

