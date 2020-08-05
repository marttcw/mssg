#include "templates.h"

#include <stdio.h>
#include <string.h>

static int32_t
templates_variable(const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	return 0;
}

static int32_t
templates_loop(const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	return 0;
}

static int32_t
templates_end(const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	return 0;
}

static int32_t
templates_string(const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	return 0;
}

static int32_t
templates_number(const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	return 0;
}

struct templates_type_info {
	const char 	*keyword;
	const int32_t	max_params;
	int32_t		(* const func)(const uint32_t, const char **);
};

static const struct templates_type_info templates_table[TEMPLATE_TOTAL] = {
	// enum templates_type	const char *	int32_t	function
	// 			keyword		max_p.
	[TEMPLATE_NOT_FOUND] 	= { NULL,	0,	NULL },
	[TEMPLATE_ROOT] 	= { NULL,	0,	NULL },
	[TEMPLATE_VARIABLE] 	= { NULL,	0,	templates_variable },
	[TEMPLATE_LOOP] 	= { "loop",	4,	templates_loop },
	[TEMPLATE_END]		= { "end",	1,	templates_end },
	[TEMPLATE_SETTER_STR] 	= { "string", 	3,	templates_string },
	[TEMPLATE_SETTER_NUM] 	= { "number", 	3,	templates_number },
};

enum templates_type
templates_str_to_type(const char *keyword)
{
	for (uint32_t i = 0; i < TEMPLATE_TOTAL; ++i)
	{
		if (templates_table[i].keyword != NULL &&
				!strcmp(templates_table[i].keyword, keyword))
		{
			return i;
		}
	}

	return TEMPLATE_NOT_FOUND;
}

const char *
templates_type_to_str(const enum templates_type type)
{
	return templates_table[type].keyword;
}

int32_t
templates(const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	return 0;
}

