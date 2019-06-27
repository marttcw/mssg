#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "mssg.h"
#include "messages.h"

/*
 * Generate the site itself
 */
int
generate(int argc, char **argv, int flag)
{
	FILE	*fp;
	char	*line = NULL;
	size_t	len = 0;
	ssize_t	nread;

	int	inpara = 0;

	// Read configuration file to get parameters

	fp = fopen("index.mssf", "r");
	if (fp == NULL) {
		fprintf(stderr, "%s: error: index.mssf not found, check if"
				" you're in the site's directory\n", TSH);
		return EXIT_FAILURE;
	}

	printf("<!DOCTYPE html><html><head></head><body>\n");
	while ((nread = getline(&line, &len, fp)) != -1) {
		printf("%s", mdline_to_html(line, &inpara));
	}
	printf("</body></html>\n");

	free(line);
	fclose(fp);
	return EXIT_SUCCESS;
}

int
create_dir(const char *dirname, mode_t mode)
{
	if (mkdir(dirname, 0755) == -1) {
		fprintf(stderr, "%s: '%s' already exists\n",
				TSH, dirname);
		return -1;
	} else {
		printf("Directory '%s' generated\n",
				dirname);
		return 0;
	}
}

/* 
 * Generate a new static site setup
 */
int
new_generate(int argc, char **argv, int flag)
{
	const char dirnames[6][16] = {
		"", "site", "blog", "template", "files", "pages"
	};

	const unsigned short FGEN_ARR_SIZE = 2;
	const char fnames[][16] = {
		CONF_NAME, "index.mssf"
	};
	char (*filegens)[FGEN_ARR_SIZE] =
		malloc(sizeof(char [FGEN_ARR_SIZE][FGEN_LEN]));

	int ret_val = EXIT_SUCCESS;
	const char *name = argv[2];
	char *nextfile = (char *) malloc((strlen(name) + 16) *
			(sizeof(char *)));
	FILE *fp = NULL;

	printf("Generating '%s'\n", name);

	/* Create directories */
	for (int i=0; i < 6; ++i) {
		sprintf(nextfile, "%s/%s", name, dirnames[i]);
		if (create_dir(nextfile, 0755) == -1) {
			ret_val = errno;
			goto NEWGEN_FAIL_DIR;
		}
	}

	/* Create configuration files */
	sprintf(filegens[0], 	"name %s\n"
				"min_compat %d\n"
				"template \n"
				"\n"
				, name, MIN_COMPAT);

	sprintf(filegens[1],	"# %s\n"
				"%%%%NAV\n"
				"## Homepage\n"
				"Hello, this is the front page!\n"
				"%%%%BLOG_FRONT(3)\n"
				"\n"
		       		, name);

	for (int i=0; i < FGEN_ARR_SIZE; ++i) {
		sprintf(nextfile, "%s/%s", name, fnames[i]);
		if ((fp = fopen(nextfile, "w")) == NULL) {
			fprintf(stderr, "%s: '%s' cannot be created\n",
					TSH, nextfile);
			ret_val = errno;
			goto NEWGEN_FAIL_FOPEN;
		} else {
			fprintf(fp, filegens[i]);
			fclose(fp);
		}
	}

	/* goto chain error handling */
NEWGEN_FAIL_FOPEN:
	free(filegens);

NEWGEN_FAIL_DIR:
	free(nextfile);
	return ret_val;
}

