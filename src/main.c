#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "states.h"

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
readfile(const char *filepath)
{
	state s;

	state_init(&s);
	state_set_level_file(&s, filepath);
	state_start_generate(&s);
	state_destroy(&s);

	putchar('\n');

	return 0;
}

int
main(int argc, char **argv)
{
	if (argc > 1) {
		if (readfile(argv[1]) < 0) {
			return EXIT_FAILURE;
		}
	} else {
		fprintf(stderr, "A parameter must be given.\n");
		print_help();
	}

	return EXIT_SUCCESS;
}

