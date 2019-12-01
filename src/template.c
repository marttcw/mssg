#include "template.h"
#include "states.h"

#include <stdio.h>
#include <string.h>

int
template_base(state *s, int argc, char **argv, int flag)
{
	// Unused variables
	(void)(flag);

	if (argc <= 1) {
		fprintf(stderr, "Error: Template: base - need 2 arguments: base {filepath}");
		return -1;
	}

	// TODO
	//
	// Different data type?
	// Make sure file return back to base file
	// EX:
	// 	index.html read
	// 	template read
	// 	index.html cont-read
	// 	template cont-read
	state_set_level_file(s, argv[1]);

	return 0;
}

int
template_string(state *s, int argc, char **argv, int flag)
{
	// Unused variables
	(void)(flag);

	if (argc <= 2) {
		fprintf(stderr, "Error: Template: text - need 3 arguments: text {name} {string}");
		return -1;
	}

	strcpy(s->variables_list[s->var_l_m].name, argv[1]);
	strcpy(s->variables_list[s->var_l_m].value, argv[2]);
	s->variables_list[s->var_l_m].type = STR;

	++s->var_l_m;

	return 0;
}

int
template_sub_content(state *s, int argc, char **argv, int flag)
{
	// Unused variables
	(void)(s);
	(void)(argc);
	(void)(argv);
	(void)(flag);

	return 0;
}

