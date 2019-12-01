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

	state_set_bef_level_file(s, argv[1]);
	state_level_down(s);

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
	(void)(flag);
	(void)(argc);
	(void)(argv);

	// TODO
	//
	// Different data type?
	// Make sure file return back to base file
	// EX:
	// 	index.html read
	// 	template read
	// 	index.html cont-read
	// 	template cont-read
	state_level_up(s);
	printf("LEVEL GO UP\n");
	
	return 0;
}

