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
	VARADD_DICT = 3,
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
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "Error: '%s' (%u) Template: extends - need 2 arguments: extends {filepath}\n"
						, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
				s->new_err = 1;
			}
			break;
		case EXTENDS_BASE:
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "Error: '%s' (%u) Template: base - need 2 arguments: base {filepath}\n"
						, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
				s->new_err = 1;
			}
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
		if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
			fprintf(stderr, "Error: '%s' (%u) Template: block - need 2 arguments: block {name} then {content} then end with endblock\n"
					, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
			s->new_err = 1;
		}
		return -2;
	} else if (flag == VARADD_STR && argc <= 2) {
		if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
			fprintf(stderr, "Error: '%s' (%u) Template: string - need 3 arguments: string {name} {string}\n"
					, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
			s->new_err = 1;
		}
		return -1;
	} else if (flag == VARADD_LIST && argc <= 2) {
		if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
			fprintf(stderr, "Error: '%s' (%u) Template: list - need 3 arguments at least: list {name} {item 0} {item 1}...\n"
					, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
			s->new_err = 1;
		}
		return -3;
	}

#ifdef DEBUG
	printf("template_variable_add\n");
#endif

	if (flag == VARADD_STR || flag == VARADD_BLOCK) {
		var_string *new_vi = calloc(1, sizeof(var_string));
		new_vi->value = calloc(strlen(argv[2]), sizeof(char));

		strcpy(new_vi->value, argv[2]);
		new_vi->type = STR;
		// If it is a configuration file - GLOBAL ensures it get kept after file scope out
		if (s->fpsc_l[s->fp_l_level].type == 1) {
			new_vi->flag = GLOBAL;
		} else {
			new_vi->flag = LOCAL;
		}

		hashmap_setValue(s->variables_hm, argv[1], new_vi, sizeof(var_string), STR);
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
	} else if (flag == VARADD_DICT) {
		char key[256];
		char value[512];
		var_dict *new_dict = calloc(1, sizeof(var_dict));
		new_dict->hashmap = hashmap_new(10);
		for (int i=2; i < argc; ++i) {
			if (sscanf(argv[i], "%[^:]:%[^\n]", key, value) == 2) {
				hashmap_setValue(new_dict->hashmap, key, value, (strlen(value)+1) * sizeof(char), STR);
			} else {
				if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
					fprintf(stderr, "Error: '%s' (%u) Template: Dictionary: Cannot get key value from '%s'.\n"
							, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line, argv[i]);
					s->new_err = 1;
				}
				hashmap_destroy(new_dict->hashmap);
				free(new_dict);
				return -1;
			}
		}

		// If it is a configuration file - GLOBAL ensures it get kept after file scope out
		if (s->fpsc_l[s->fp_l_level].type == 1) {
			new_dict->flag = GLOBAL;
		} else {
			new_dict->flag = LOCAL;
		}

		hashmap_setValue(s->variables_hm, argv[1], new_dict, sizeof(var_dict), DICT);
	}

	if (flag == VARADD_STR || flag == VARADD_LIST || flag == VARADD_DICT) {
		s->fpsc_l[s->fp_l_level].sc.current_state = COPY;
	}

	return 0;
}

int
template_content(state *s, int argc, char **argv, int flag)
{
	if (flag == CONTENTS) {
		if (argc <= 1) {
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "Error: '%s' (%u) Template: base - need 2 arguments: content {filepath}\n"
						, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
				s->new_err = 1;
			}
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
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "Error: '%s' (%u) Template: link - need 2 arguments: link {filepath}\n"
						, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
				s->new_err = 1;
			}
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
	static var_string *vi;

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
			if ((argc > 4) && (sscanf(argv[4], "%d", &range) != 1)) {
				if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
					fprintf(stderr, "template: for: range set not a number\n");
					s->new_err = 1;
				}
				return -1;
			}
			if ((argc > 5) && (sscanf(argv[5], "%d", &increment) != 1)) {
				if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
					fprintf(stderr, "template: for: increment set not a number\n");
					s->new_err = 1;
				}
				return -1;
			}
			if ((argc > 6) && (sscanf(argv[6], "%u", &start) != 1)) {
				if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
					fprintf(stderr, "template: for: start set not a positive number\n");
					s->new_err = 1;
				}
				return -1;
			}

			vl = hashmap_getValue(s->variables_hm, list);
			if (vl == NULL) {
				if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
					fprintf(stderr, "List variable: '%s' not found\n", list);
					s->new_err = 1;
				}
				return -1;
			}

			if (range < 0) {
				range = vl->size;
			} else if ((unsigned int) range > vl->size) {
				if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
					fprintf(stderr, "warning: template: for: range out of range: %u > %u | Setting range as range.\n", range, vl->size);
					s->new_err = 1;
				}
				range = vl->size;
			} else if (start > vl->size) {
				if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
					fprintf(stderr, "template: for: start out of range: %u > %u\n", start, vl->size);
					s->new_err = 1;
				}
				return -1;
			}

			current_index = start;
			file_position = ftell(s->fpsc_l[s->fp_l_level].fp);
#ifdef DEBUG
			printf("set: %ld %d %d %d %s %s\n", file_position, range, increment, current_index, item, list);
#endif
			vi = calloc(1, sizeof(var_string));
			vi->type = STR;
			vi->flag = LOCAL;
			vi->value = calloc(1024, sizeof(char));
			strcpy(vi->value, vl->list[current_index]);
			hashmap_setValue(s->variables_hm, item, vi, sizeof(var_string), STR);
		} else if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
			fprintf(stderr, "template: for: must have at least 3 parameters: EX: for item in list (times) (increment)\n");
			s->new_err = 1;
			return -1;
		} else {
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
				hashmap_setValue(s->variables_hm, item, vi, sizeof(var_string), STR);
#ifdef DEBUG
				printf("loop: %ld %d %d %d %s %s\n", file_position, range, increment, current_index, item, list);
#endif
				fseek(s->fpsc_l[s->fp_l_level].fp, file_position, SEEK_SET);
			}
		} else {
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "'%s' (%u): template: for loop have not been set before the endfor.\n"
						, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
				s->new_err = 1;
			}
			return -2;
		}
	}

	return 0;
}

int
template_copy(state *s, int argc, char **argv, int flag)
{
	(void)(flag);

	// Allocate memory
	if (s->copy_list == NULL) {
		s->copy_list = calloc(argc - 1, sizeof(char *));
	} else {
		s->copy_list = realloc(s->copy_list, (s->copy_list_max + (argc - 1)) * sizeof(char *));
	}

	for (int i=1; i < argc; ++i, ++s->copy_list_max) {
		s->copy_list[s->copy_list_max] = calloc(strlen(argv[i]) + 1, sizeof(char));
		strcpy(s->copy_list[s->copy_list_max], argv[i]);
	}

	return 0;
}

int
template_execute(state *s, int argc, char **argv, int flag)
{
	(void)(s);
	(void)(flag);

	char *cmd = calloc(1280, sizeof(char));

	for (int i=1; i < argc; ++i) {
		strcat(cmd, argv[i]);
		strcat(cmd, " ");
	}

	// Execute command
	system(cmd);

	free(cmd);
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
		{"for",		&template_for,		FOR},		// {% for item in items -1 %}
		{"endfor",	&template_for,		FOR_END},	// {% endfor %}
		{"list",	&template_variable_add,	VARADD_LIST},	// {% list foo apple banana carrot %}
		{"dict",	&template_variable_add,	VARADD_DICT},	// {% dict fruit name:apple texture:crunchy %}
		{"copy",	&template_copy,		-1},		// {% copy style.css rss.xml %}
		{"execute",	&template_execute,	-1},		// {% execute mogrify -path build/img_thumb -thumbnail 250x250 src/img_original/*.jpg %}
		/* Empty templates, configuration file only, these are processed seperately */
		{"blog",	&template_none,		-1},		// blog - config file only
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
	if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
		fprintf(stderr, "'%s' (%u): Template: '%s' not found: Mis-spelling\n"
				, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line, s->keywords_list[0]);
		s->new_err = 1;
	}
	return -3;
}

char *
template_variable_get_value(state *s, const char *var_name, const int *type, const void *index, const int *s_type)
{
	(void)(s_type);	// unused

	if (*type == STR) {
		var_string *var = hashmap_getValue(s->variables_hm, var_name);
		return var->value;
	} else if (*type == LIST) {
		var_list *var = hashmap_getValue(s->variables_hm, var_name);
		if (*(unsigned int *)index < var->size) {
			return var->list[*(unsigned int *)index];
		} else {
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "Variable: %s: Index %u is out of range: 0 to %u.\n", var_name, *(unsigned int *)index, var->size);
				s->new_err = 1;
			}
			return NULL;
		}
	} else if (*type == DICT) {
		var_dict *var = hashmap_getValue(s->variables_hm, var_name);
		char *value = hashmap_getValue(var->hashmap, (char *) index);
		if (value == NULL) {
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "'%s' (%d): Sub variable: '%s' (%s | %s) not found: Mis-spelling or not defined before usage\n"
						, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line, (char *) index, var_name, s->variable);
				s->new_err = 1;
			}
			return NULL;
		} else {
			return value;
		}
	} else {
		fprintf(stderr, "'%s' (%d): Unknown error\n"
				, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line);
		return NULL;
	}
}

/* Variables templates handling
 * EX: {{ example }} {{ example.sub }} {{ example[x] }}
 */
int
template_variable(state *s)
{
	int *type = NULL;
	char var_name[256], sub_name[128];
	int s_type = 0;
	unsigned int index;
	char *value = NULL;
	char var_name_temp[256];
	strcpy(var_name, s->variable);

	do {
		// Test the string
		if (sscanf(var_name, "%[^.].%s", var_name_temp, sub_name) == 2) {
			s_type = 2;	// Dictionary
			strcpy(var_name, var_name_temp);
		} else if (sscanf(var_name, "%[^[][%u]", var_name_temp, &index) == 2) {
			s_type = 1;	// List
			strcpy(var_name, var_name_temp);
		} else {
			s_type = 0;	// String or pointer
		}

		type = hashmap_getType(s->variables_hm, var_name);

		if (type == NULL) {
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "'%s' (%d): Variable: '%s' (%s) not found: Mis-spelling or not defined before usage\n"
						, s->fpsc_l[s->fp_l_level].filename, s->fpsc_l[s->fp_l_level].line, var_name, s->variable);
				s->new_err = 1;
			}
			return -1;
		}

		if (*type == STR) {
			value = template_variable_get_value(s, var_name, type, NULL, &s_type);
		} else if (*type == LIST) {
			value = template_variable_get_value(s, var_name, type, &index, &s_type);
		} else if (*type == DICT) {
			value = template_variable_get_value(s, var_name, type, &sub_name, &s_type);
		} else {
			if (hashmap_getValue(s->errors_hm, s->fname_line) == NULL) {
				fprintf(stderr, "Unsupported type for variable: '%s' | Type: %d\n", var_name, *type);
				s->new_err = 1;
			}
			return -2;
		}

		if (value == NULL) {
			return -1;
		}

	} while (sscanf(value, "$%s", var_name) == 1);

	fprintf(s->fp_o, "%s", value);
	return 0;
}

