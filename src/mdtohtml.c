#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "mssg.h"
#include "genproc.h"
#include "messages.h"

#define LINE_BUFFER (1024)

char *
gen_from_proc(char *proc_type)
{
	char *proc_name = NULL;
	char *proc_arg = NULL;
	char *token = NULL;

	/* Gets the pre-processor name */
	token = strtok(proc_type, "(");
	proc_name = (char *) malloc(strlen(proc_type) * sizeof(char));
	sprintf(proc_name, "%s", token);

	/* Tests if the arguments are used or not */
	token = strtok(NULL, ")");
	if (token != NULL) {
		proc_arg  = (char *) malloc(strlen(proc_type) * sizeof(char));
		sprintf(proc_arg, "%s", token);
	}

	typedef struct func_m {
		char *name;
		char *(*f)(char *);
		int min_args;
	} func_m;

	func_m *funcs = (func_m []) {
		/* name,	function,	min_args */
		{"NAV",		&gen_nav,	0},
		{"BLOG_FRONT",	&gen_blog,	0},
		{NULL,		NULL,		0}
	};

	func_m *fp = funcs;	// Pointer to function matcher

	/* Looping and matching between name and its function */
	for (; fp->name != NULL; ++fp) {
		if (!strcmp(fp->name, proc_name)) {
			return (*fp->f)(proc_arg);
		}
	}

	if (token != NULL)	{ free(token); }
	if (proc_name != NULL)	{ free(proc_name); }
	if (proc_arg != NULL)	{ free(proc_arg); }

	return NULL;	// Function not found
}

int
preproc(const char *line, char *htmlline, const int inpara)
{
	char *proc_type = (char *) malloc(strlen(line) * sizeof(char));
	char *gen_proc_str = NULL;

	if (line[1] != '%') {
		sprintf(htmlline, "%s", (inpara) ? "</p>" : "");
	} else {
		sprintf(proc_type, "%.*s", (int) (strlen(line+2)-1), line+2);
		sprintf(htmlline, "%s%s", (inpara) ? "</p>" : "",
				((gen_proc_str = gen_from_proc(proc_type))
				 	== NULL) ? "" : gen_proc_str);
	}

	if (gen_proc_str != NULL)	{ free(gen_proc_str); }
	if (proc_type != NULL)		{ free(proc_type); }
	return 0;
}

int
newline(const char *line, char *htmlline, const int inpara)
{
	sprintf(htmlline, "%s", (inpara) ? "</p>" : "");
	return 0;
}

int
header(const char *line, char *htmlline, const int inpara)
{
	int ret_val = 0;
	int hcounter = 1;

	while (line[hcounter] == '#') {
		++hcounter;
	}
	if (hcounter > 6) {
		fprintf(stderr, "%s: warning: Header level too high,"
				" setting to h6\n", TSH);
		hcounter = 6;
		ret_val = 1;
	}
	sprintf(htmlline, "%s<h%d>%.*s</h%d>\n",
			(inpara) ? "</p>" : "",
			hcounter,
			(int) strlen(line+hcounter)-1, line+hcounter,
			hcounter);
	return ret_val;
}

char *
mdline_to_html(const char *line, int *inpara)
{
	int func_used = 0;

	typedef struct func_m {
		char symbol;
		int (*f)(const char *, char *, const int);
	} func_m;

	func_m *funcs = (func_m []) {
		{'#',	&header},
		{'\n',	&newline},
		{'%',	&preproc},
		{'\0',	NULL}
	};

	func_m *fp = funcs;

	char *htmlline = (char *) malloc(sizeof(char) *
			(strlen(line) + LINE_BUFFER));

	for (; fp->symbol != '\0'; ++fp) {
		if (line[0] == fp->symbol) {
			(*fp->f)(line, htmlline, *inpara);
			*inpara = 0;
			func_used = 1;
			break;
		}
	}

	// Paragraph
	if (!func_used) {
		sprintf(htmlline, "<p>\n%s", line);
		*inpara = 1;
	}

	return htmlline;
}


