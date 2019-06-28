#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "genproc.h"

#define LINE_BUFFER (1024)

/*
 * Generates the navigation bar
 */
char *
gen_nav(char *param)
{
	char *generated = (char *) malloc(LINE_BUFFER * sizeof(char));
	DIR		*d;
	struct dirent	*dir;
	char *temp = (char *) malloc(LINE_BUFFER * sizeof(char));

	/* Error occurred */
	if (generated == NULL) {
		return NULL;
	}

	sprintf(generated, "<nav>"
			"<a href=\"/\">Home</a> <a href=\"/blog/\">Blog</a>");

	if (chdir("pages") == -1) {
		// Error occurred
		fprintf(stderr, "%s: error: Directory 'pages' not found\n",
				TSH);
	} else {
		d = opendir(".");
		if (d) {
			// Loop through files
			while ((dir = readdir(d)) != NULL) {
				if (strcmp(dir->d_name, ".") &&
						strcmp(dir->d_name,"..")) {
					sprintf(temp, " <a href=\"/\"%s\"/\">"
							"%s</a>"
							, dir->d_name
							,dir->d_name);
					strcat(generated, temp);
				}
			}
			closedir(d);
		} else {
			fprintf(stderr, "%s: error: Directory 'pages' not"
					" opened\n"
					, TSH);
		}
	}
	strcat(generated, "</nav>\n");
	free(temp);

	return generated;
}

/*
 * Generates the most recent blog posts
 */
char *
gen_blog(char *param)
{
	char *generated = (char *) malloc(LINE_BUFFER * sizeof(char));

	sprintf(generated, "<h1>Blog PLACEHOLDER</h1>\n");

	return generated;
}

