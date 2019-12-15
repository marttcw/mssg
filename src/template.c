#include "template.h"
#include "states.h"

#include <stdio.h>
#include <string.h>

//#define DEBUG

typedef struct func_matcher {
	char *str;
	int (*f)(state *s, int, char **, int);
	int flag;
} func_matcher;

int template_base(state *s, int argc, char **argv, int flag);
int template_string(state *s, int argc, char **argv, int flag);
int template_sub_content(state *s, int argc, char **argv, int flag);
int template_content(state *s, int argc, char **argv, int flag);
int template_link(state *s, int argc, char **argv, int flag);
int template_link_var(state *s, int argc, char **argv, int flag);

int
template_base(state *s, int argc, char **argv, int flag)
{
	// Unused variables
	(void)(flag);

	if (argc <= 1) {
		fprintf(stderr, "Error: Template: base - need 2 arguments: base {filepath}");
		return -1;
	}

	state_set_bef_level_file(s, argv[1], 0);
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

	state_level_up(s);
	
	return 0;
}

int
template_content(state *s, int argc, char **argv, int flag)
{
	// Unused variables
	(void)(flag);

	if (argc <= 1) {
		fprintf(stderr, "Error: Template: base - need 2 arguments: base {filepath}");
		return -1;
	}

	state_set_level_file(s, argv[1], 0);
	state_level_up(s);

	return 0;
}

int
template_link(state *s, int argc, char **argv, int flag)
{
	int i;
	char src_prefix[] = "src/";

	// Unused variables
	(void)(flag);

	if (argc <= 1) {
		fprintf(stderr, "Error: Template: link - need 2 arguments: link {filepath}");
		return -1;
	}

	for (i = 0; argv[1] == src_prefix; ++i) {
	}

	fprintf(s->fp_o, "%s", argv[1]+(i));

	return 0;
}

// TODO
int
template_link_var(state *s, int argc, char **argv, int flag)
{
	// Unused variables
	(void)(flag);
	(void)(argc);
	(void)(argv);

	return template_link(s, 2, argv, -1);
}

void
print_keywords_list(state *s)
{
	printf("TEMPLATE: ");
	for (unsigned int i=0; i <= s->keyword_i; ++i) {
		printf("\"%s\"  ", s->keywords_list[i]);
	}
	putchar('\n');
}

/* Special templates handling
 * EX: {% example options %}
 */
int
template_keywords_list(state *s)
{
	++s->keyword_i;

	// Discard last item if last item is empty
	if (s->keywords_list[s->keyword_i][0] == '\0') {
		--s->keyword_i;
	}

#ifdef DEBUG
	print_keywords_list(s);
#endif

	func_matcher *funcs = (func_matcher []) {
		/* 1st param,		function,		flag	// Example */
		{"base", 		&template_base, 	-1},	// {% base src/dir/foo.html %}
		{"string", 		&template_string, 	-1},	// {% string foo "hello world" %}
		{"SUB_CONTENT", 	&template_sub_content, 	-1},	// {% SUB_CONTENT %}
		{"content",		&template_content,	-1},	// {% content src/dir/foo.html %}
		{"link",		&template_link, 	-1},	// {% link src/dir/foo.css %}
		{"link_var",		&template_link_var, 	-1},	// {% link_var foo %}
		{NULL, 			NULL,			-2}
	};

	/* Pointer variable of the function matcher */
	func_matcher *ptr = funcs;

	// Looping and matching through parameters to its function
	for (; ptr->str != NULL && s->keyword_i > 0; ptr++) {
		if (!strcmp(ptr->str, s->keywords_list[0])) {
			return (*ptr->f)(s, (s->keyword_i + 1),
					s->keywords_list, ptr->flag);
		}
	}

	// No arguments found
	fprintf(stderr, "Error: Template: No arguments found\n");
	return -3;
}

/* Variables templates handling
 * EX: {{ example }}
 */
int
template_variable(state *s)
{
	for (unsigned int i=0; i < s->var_l_m; ++i) {
		if (!strcmp(s->variables_list[i].name, s->variable)) {
			fprintf(s->fp_o, "%s", s->variables_list[i].value);
			return 0;
		}
	}

	return -1;
}

