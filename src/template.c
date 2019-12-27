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
	VARADD_BLOCK = 1,
	VARADD_LIST = 2,
	FOR = 0,
	FOR_END = 1
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
	} else if (flag == VARADD_STR && argc <= 2) {
		fprintf(stderr, "Error: Template: string - need 3 arguments: string {name} {string}\n");
		return -1;
	} else if (flag == VARADD_LIST && argc <= 2) {
		fprintf(stderr, "Error: Template: list - need 3 arguments at least: list {name} {item 0} {item 1}...\n"); 
		return -3;
	}

#ifdef DEBUG
	printf("template_variable_add\n");
#endif

	if (flag == VARADD_STR || flag == VARADD_BLOCK) {
		var_info *new_vi = calloc(1, sizeof(var_info));
		new_vi->value = calloc(strlen(argv[2]), sizeof(char));

		strcpy(new_vi->value, argv[2]);
		new_vi->type = STR;
		// If it is a configuration file - GLOBAL ensures it get kept after file scope out
		if (s->fpsc_l[s->fp_l_level].type == 1) {
			new_vi->flag = GLOBAL;
		} else {
			new_vi->flag = LOCAL;
		}

		hashmap_setValue(s->variables_hm, argv[1], new_vi, sizeof(var_info), STR);
	} else if (flag == VARADD_LIST) {
		var_list *new_vl = calloc(1, sizeof(var_list));
		new_vl->size = argc - 2;
		new_vl->list = calloc(new_vl->size + 1, sizeof(char *));
		for (int i=2; i < argc; ++i) {
			new_vl->list[i-2] = calloc(strlen(argv[i]) + 1, sizeof(char));
			strcpy(new_vl->list[i-2], argv[i]);
		}
		// If it is a configuration file - GLOBAL ensures it get kept after file scope out
		if (s->fpsc_l[s->fp_l_level].type == 1) {
			new_vl->flag = GLOBAL;
		} else {
			new_vl->flag = LOCAL;
		}

		hashmap_setValue(s->variables_hm, argv[1], new_vl, sizeof(var_list), LIST);
	}

	if (flag == VARADD_STR || flag == VARADD_LIST) {
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

/*
 * {% for item in list {range} {increment} {start} %}
 */
int
template_for(state *s, int argc, char **argv, int flag)
{
	// Saved variables
	static long file_position;
	static int range, increment, current_index;
	static char *item, *list;
	static var_list *vl;
	static var_info *vi;

	if (flag == FOR) {
		// Set defaults
		file_position = -1;
		range = -1;
		increment = 1;
		current_index = 0;
		item = calloc(strlen(argv[1]) + 1, sizeof(char));
		list = calloc(strlen(argv[3]) + 1, sizeof(char));
		vl = NULL;
		unsigned int start = 0;

		if (argc > 3) {
			strcpy(item, argv[1]);
			strcpy(list, argv[3]);
			if (argc > 4) {
				if (sscanf(argv[4], "%d", &range) != 1) {
					fprintf(stderr, "template: for: range set not a number\n");
					return -1;
				}
			}
			if (argc > 5) {
				if (sscanf(argv[5], "%d", &increment) != 1) {
					fprintf(stderr, "template: for: increment set not a number\n");
					return -1;
				}
			}
			if (argc > 6) {
				if (sscanf(argv[6], "%u", &start) != 1) {
					fprintf(stderr, "template: for: start set not a positive number\n");
					return -1;
				}
			}

			vl = hashmap_getValue(s->variables_hm, list);
			if (vl == NULL) {
				fprintf(stderr, "List variable: '%s' not found\n", list);
				return -1;
			}

			if (range < 0) {
				range = vl->size;
			} else if ((unsigned int) range > vl->size) {
				fprintf(stderr, "warning: template: for: range out of range: %u > %u | Setting range as range.\n", range, vl->size);
				range = vl->size;
			} else if (start > vl->size) {
				fprintf(stderr, "template: for: start out of range: %u > %u\n", start, vl->size);
				return -1;
			}

			current_index = start;
			file_position = ftell(s->fpsc_l[s->fp_l_level].fp);
#ifdef DEBUG
			printf("set: %ld %d %d %d %s %s\n", file_position, range, increment, current_index, item, list);
#endif
			vi = calloc(1, sizeof(var_info));
			vi->type = STR;
			vi->flag = LOCAL;
			vi->value = calloc(1024, sizeof(char));
			strcpy(vi->value, vl->list[current_index]);
			hashmap_setValue(s->variables_hm, item, vi, sizeof(var_info), STR);
		} else {
			fprintf(stderr, "template: for: must have at least 3 parameters: EX: for item in list (times) (increment)\n");
			return -1;
		}
	} else if (flag == FOR_END) {
		if (file_position != -1) {
			if (current_index == (range - 1)) {
				hashmap_removeKey(s->variables_hm, item);
				file_position = -1;
			} else {
				current_index += increment;
				strcpy(vi->value, vl->list[current_index]);
				hashmap_setValue(s->variables_hm, item, vi, sizeof(var_info), STR);
#ifdef DEBUG
				printf("loop: %ld %d %d %d %s %s\n", file_position, range, increment, current_index, item, list);
#endif
				fseek(s->fpsc_l[s->fp_l_level].fp, file_position, SEEK_SET);
			}
		} else {
			fprintf(stderr, "for loop have not been set before the endfor.\n");
			return -2;
		}
	}

	return 0;
}

int
template_none(state *s, int argc, char **argv, int flag)
{
	(void)(s);
	(void)(argc);
	(void)(argv);
	(void)(flag);

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
		{"blog",	&template_none,		-1},		// blog - config file only
		{"for",		&template_for,		FOR},		// {% for item in items -1 %}
		{"endfor",	&template_for,		FOR_END},		// {% endfor %}
		{"list",	&template_variable_add,	VARADD_LIST},	// {% list foo apple banana carrot %}
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
 * EX: {{ example }} {{ example.sub }} {{ example[x] }}
 */
int
template_variable(state *s)
{
	int *type = NULL;
	char var_name[128], sub_name[128];
	int s_type = 0;
	unsigned int index;

	// Test the string
	if (sscanf(s->variable, "%s.%s", var_name, sub_name) == 2) {
		s_type = 1;
	} else if (sscanf(s->variable, "%s[%u]", var_name, &index) == 2) {
		s_type = 2;
	} else {
		s_type = 0;
		strcpy(var_name, s->variable);
	}

	type = hashmap_getType(s->variables_hm, var_name);

	if (type == NULL) {
		fprintf(stderr, "'%s' (%d): Variable: '%s' (%s) not found: Mis-spelling or not defined before usage\n"
				, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line, var_name, s->variable);
		return -1;
	}

	if (*type == STR && s_type == 0) {
		var_info *var = hashmap_getValue(s->variables_hm, s->variable);
		fprintf(s->fp_o, "%s", var->value);
	} else if (*type == LIST && s_type == 2) {
		var_list *var = hashmap_getValue(s->variables_hm, var_name);
		if (index < var->size) {
			fprintf(s->fp_o, "%s", var->list[index]);
		} else {
			fprintf(stderr, "Variable: %s: Index %u is out of range: 0 to %u.\n", var_name, index, var->size);
			return -3;
		}
	} else {
		fprintf(stderr, "Unsupported type for variable: '%s' | Type: %d\n", var_name, *type);
		return -2;
	}

	return 0;
}

