#include "blog.h"

#include "extra.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *
title2path(const char *title)
{
	unsigned int title_len = strlen(title);
	char *path = calloc(title_len, sizeof(char));
	unsigned int i;

	for (i = 0; i < title_len; ++i) {
		switch (title[i]) {
		case ' ': case '\'': case '/':
			path[i] = '-';
			break;
		default:
			path[i] = title[i];
		}
	}

	return path;
}

char *
blog_quick_config(void)
{
	char line[512];
	char *token = NULL;
	const char delim[] = " ";
	char *execute = calloc(256, sizeof(char));

	// Default
	strcpy(execute, "vim");

	FILE *fp = fopen("config", "r");

	if (fp == NULL) {
		return execute;
	}

	while (fgets(line, 512, fp) != NULL) {
		token = strtok(line, delim);
		// If first parameter is blog
		if (!strcmp(token, "blog")) {
			token = strtok(NULL, delim);

			if (!strcmp(token, "editor")) {
				token = strtok(NULL, "\n ");
				strcpy(execute, token);
				printf("'%s' used for editing\n", execute);
			} else {
				fprintf(stderr, "Error: blog: config: '%s' parameters given not found\n", line);
			}
		}
	}

	fclose(fp);

	return execute;
}

void
blog_new(const char *date_string, const char *title)
{
	unsigned int src_strlen = 64 + strlen(title);
	char src_path[src_strlen], src_file[src_strlen + 16];
	date *d = date_new(date_string);

	if (d == NULL) {
		// Failed boundaries checking
		return;
	}

	// Boundaries checking passed, create source file from it
	sprintf(src_path, "src/blog/%4d/%02d/%02d/%s", d->year, d->month, d->day, title2path(title));
	sprintf(src_file, "%s/index.html", src_path);

	if (m_mkdir(src_path, 0777) < 0) {
		// If the error is not directory already exists
		if (errno != 17) {
			perror("Error: Directory creation error: ");
		}
	} else {
		FILE *fp = fopen(src_file, "w");

		fprintf(fp, "{%% base src/base.html %%}\n"
				"<h1>%s</h1>\n"
				"<p>%d-%d-%d</p>\n"
				"Blog template\n"
				, title, d->year, d->month, d->day);

		fclose(fp);
	}

	printf("Blog html file created at: '%s'\n", src_file);

	date_destroy(d);
}

void
blog_edit(const char *date_string, const char *title)
{
	unsigned int src_strlen = 64 + strlen(title);
	char src_path[src_strlen], src_file[src_strlen + 16], src_edit[src_strlen + 64];
	char *execute = blog_quick_config();
	date *d = date_new(date_string);

	if (d == NULL) {
		// Failed boundaries checking
		free(execute);
		return;
	}

	// Boundaries checking passed, create source file from it
	sprintf(src_path, "src/blog/%4d/%02d/%02d/%s", d->year, d->month, d->day, title2path(title));
	sprintf(src_file, "%s/index.html", src_path);
	sprintf(src_edit, "%s %s", execute, src_file);

	FILE *fp = fopen(src_file, "r");
	if (fp == NULL) {
		// Return early, file doesn't exists
		fprintf(stderr, "Error: Blog file path '%s' does not exists.\n", src_file);
	} else {
		// Found, and edit it
		fclose(fp);
		system(src_edit);
	}

	date_destroy(d);
	free(execute);
}

