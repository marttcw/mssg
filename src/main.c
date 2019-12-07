#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "states.h"
#include "files.h"

#define VERSION "v0.0.1"
#define DEV_BUILD

void
print_help(void)
{
	printf("mssg HELP\n\n"
			"init    Initialise a website\n"
			"build   Build the website\n"
#ifdef DEV_BUILD
			"test    Test a file (DEV BUILD)\n"
#endif
			"\n"
			"help    Print this help message\n"
			"version Print the program version\n"
			);
}

void
print_version(void)
{
	printf("mssg %s\n", VERSION);
}

int
main(int argc, char **argv)
{
	int ret = EXIT_SUCCESS;
	char cwd[256];

	files *f = files_init();

	if (argc > 1) {
		if (!strcmp(argv[1], "build")) {
			if (getcwd(cwd, sizeof(cwd)) == NULL) {
				perror("getcwd() error");
				ret = EXIT_FAILURE;
			}

			if (ret != EXIT_FAILURE && files_traverse(f, cwd) < 0) {
				ret = EXIT_FAILURE;
			}
		} else if (!strcmp(argv[1], "init")) {
			if (argc <= 2) {
				ret = init_site("");
			} else {
				ret = init_site(argv[2]);
			}
		} else if (!strcmp(argv[1], "help")) {
			print_help();
		} else if (!strcmp(argv[1], "version")) {
			print_version();
		} else {
			fprintf(stderr, "Paramter unknown.\n");
			print_help();
		}
	} else {
		fprintf(stderr, "A parameter must be given.\n");
		print_help();
	}

	files_destroy(f);
	return ret;
}

