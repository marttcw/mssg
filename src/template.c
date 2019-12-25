#include "template.h"
#include "states.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#define DEBUG

typedef struct func_matcher {
	char *str;
	int (*f)(state *s, int, char **, int);
	int flag;
} func_matcher;

enum flags {
	EXTENDS = 0,
	EXTENDS_BASE = 1,
	LINK = 0,
	CONTENTS = 0,
	CONTENTS_SUB = 1,
	VARADD_STR = 0,
	VARADD_BLOCK = 1
};

int
template_extends(state *s, int argc, char **argv, int flag)
{
#ifdef DEBUG
	printf("template_extends: argc: %d, flag: %d\n", argc, flag);
#endif
	if (argc <= 1) {
		switch (flag) {
		case EXTENDS:
			fprintf(stderr, "Error: Template: extends - need 2 arguments: extends {filepath}");
			break;
		case EXTENDS_BASE:
			fprintf(stderr, "Error: Template: base - need 2 arguments: base {filepath}");
			break;
		}
		return -1;
	}

	state_set_bef_level_file(s, argv[1], 0);
	if (flag == EXTENDS_BASE) {
		state_level_down(s);
	}

	return 0;
}

int
template_variable_add(state *s, int argc, char **argv, int flag)
{
	if (flag == VARADD_BLOCK && argc == 2) {
		s->fpsc_l[s->fp_l_level].sc.current_state = BLOCK;
		return 1;	// Do not reset keywords list
	} else if ((flag == VARADD_BLOCK) && !(argc >= 4 && !strcmp(argv[3], "endblock"))) {
		fprintf(stderr, "Error: Template: block - need 2 arguments: block {name} then {content} then end with endblock\n");
		return -2;
	}

	if (flag == VARADD_STR && argc <= 2) {
		fprintf(stderr, "Error: Template: text - need 3 arguments: text {name} {string}\n");
		return -1;
	}

#ifdef DEBUG
	printf("template_variable_add\n");
#endif

	unsigned int len = strlen(argv[2]);

	// Reallocates more memory if needed
	if (len > 320) {
		s->variables_list[s->var_l_m].value = realloc(s->variables_list[s->var_l_m].value, (len + 8) * sizeof(char));
	}

	strcpy(s->variables_list[s->var_l_m].name, argv[1]);
	strcpy(s->variables_list[s->var_l_m].value, argv[2]);
	s->variables_list[s->var_l_m].type = STR;

	// If it is a configuration file - GLOBAL ensures it get kept after file scope out
	if (s->fpsc_l[s->fp_l_level].type == 1) {
		s->variables_list[s->var_l_m].flag = GLOBAL;
	}

	++s->var_l_m;

	if (flag == VARADD_STR) {
		s->fpsc_l[s->fp_l_level].sc.current_state = COPY;
	}

	return 0;
}

int
template_content(state *s, int argc, char **argv, int flag)
{
	if (flag == CONTENTS) {
		if (argc <= 1) {
			fprintf(stderr, "Error: Template: base - need 2 arguments: content {filepath}");
			return -1;
		}

		state_set_level_file(s, argv[1], 0);
	}

	state_level_up(s);

	return 0;
}

int
template_link(state *s, int argc, char **argv, int flag)
{
	int i;
	char *src_prefix = "src/";

	if (argc <= 1) {
		switch (flag) {
		case LINK:
			fprintf(stderr, "Error: Template: link - need 2 arguments: link {filepath}");
			break;
		}
		return -1;
	}

	for (i = 0; argv[1][i] == src_prefix[i]; ++i) {
	}

	fprintf(s->fp_o, "%s", argv[1]+(i));

	return 0;
}

void
print_keywords_list(state *s)
{
	printf("TEMPLATE (%d): ", s->keyword_i);
	for (unsigned int i=0; i < s->keyword_i; ++i) {
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
		/* 1st param,	function,		flag		// Example */
		{"base", 	&template_extends, 	EXTENDS_BASE},	// {% base src/dir/foo.html %}
		{"extends",	&template_extends,	EXTENDS},	// {% extends src/dir/foo.html %}
		{"string", 	&template_variable_add,	VARADD_STR},	// {% string foo "hello world" %}
		{"sub_content", &template_content, 	CONTENTS_SUB},	// {% sub_content %}
		{"content",	&template_content,	CONTENTS},	// {% content src/dir/foo.html %}
		{"link",	&template_link, 	LINK},		// {% link src/dir/foo.css %}
		{"block",	&template_variable_add,	VARADD_BLOCK},	// {% block foo %} ... {% endblock %}
		{NULL, 		NULL,			-2}
	};

	/* Pointer variable of the function matcher */
	func_matcher *ptr = funcs;

	// Looping and matching through parameters to its function
	for (; ptr->str != NULL && s->keyword_i > 0; ptr++) {
		if (!strcmp(ptr->str, s->keywords_list[0])) {
			return (*ptr->f)(s, s->keyword_i,
					s->keywords_list, ptr->flag);
		}
	}

	// No arguments found
	fprintf(stderr, "'%s' (%d): Template: '%s' not found: Mis-spelling\n"
			, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line, s->keywords_list[0]);
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
	fprintf(stderr, "'%s' (%d): Variable: '%s' not found: Mis-spelling or not defined before usage\n"
			, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line, s->variable);

	return -1;
}

