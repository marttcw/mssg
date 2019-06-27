/* mssg - Mart's Static Site Generator
 *
 * Simple static site generator in C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mssg.h"
#include "messages.h"

int
help(int argc, char **argv, int flag)
{
	if (flag == EXIT_FAILURE && argc > 1) {
		fprintf(stderr, "%s: Invalid option: '%s'\n\n",
				TSH, argv[1]);
	}

	printf("%s\n", HELP_MESSAGE);
	return flag;
}

int
min_argc_error(char *param)
{
	fprintf(stderr, "%s: Insufficient amount of parameters given for: '%s'"
			"\n",
			TSH, param);
	return EXIT_FAILURE;
}

/* main function
 *
 * Deals with CLI parameters to its suitable function handling
 */
int
main(int argc, char **argv)
{
	/* Function matcher */
	typedef struct func_m {
		char *param;
		int (*f)(int, char **, int);
		int flag;
		int min_argc;
	} func_m;

	func_m *funcs = (func_m []) {
		/* parameter,	function,	flag,	min_argc */
		{"new",		&new_generate,	0,	3},
		{"generate",	&generate,	0,	2},
		{"help",	&help,		0,	2},
		{NULL,		NULL,		0,	0}
	};

	func_m *fp = funcs;	// Pointer to function matcher

	/* Looping and matching between parameter and its function */
	for (; fp->param != NULL && argc > 1; ++fp) {
		if (!strcmp(fp->param, argv[1])) {
			if (argc >= fp->min_argc) {
				return (*fp->f)(argc, argv, fp->flag);
			} else {
				return min_argc_error(fp->param);
			}
		}
	}

	/* Prints help/usage if command is not found */
	return help(argc, argv, EXIT_FAILURE);
}

