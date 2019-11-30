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
	FILE *fp = NULL;
	char c = '\0';
	enum e_states state = COPY;

	// -1: File not found/read error
	if ((fp = fopen(filepath, "r")) == NULL) {
		fprintf(stderr, "Error occured, cannot read file: '%s'\n", filepath);
		return -1;
	}

	// Read the file
	while (!feof(fp)) {
		c = fgetc(fp);
		if (c == -1) {
			break;
		}

		switch (state) {
		case COPY:
			state_copy(&state, &c);
			break;
		case SPEC:
			state_spec(&state, &c);
			break;
		case DET_SPEC:
			state_det_spec(&state, &c);
			break;
		case AFT_SPEC:
			state_aft_spec(&state, &c);
			break;
		default:
			fprintf(stderr, "State error\n");
			fclose(fp);
			return -2;
		}
	}
	putchar('\n');

	// Close file
	fclose(fp);

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

