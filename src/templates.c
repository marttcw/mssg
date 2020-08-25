#include "templates.h"

#include <string.h>
#include <stdlib.h>

enum vartype {
	TEMPLATE_VARTYPE_INT = 0,
	TEMPLATE_VARTYPE_DOUBLE,
	TEMPLATE_VARTYPE_STRING,
	TEMPLATE_VARTYPE_ERROR
};

struct variable {
	char		name[64];
	enum vartype	type;
	union {
		int32_t	num_int;
		double	num_double;
		char	string[256];
	} var;
};

struct variables_list {
	struct variable 	*list;
	uint32_t		length;
	uint32_t		allocated;
	const uint32_t		ALLOC_CHUNK;
};

struct variables_list variables = {
	.list = NULL,
	.length = 0,
	.allocated = 0,
	.ALLOC_CHUNK = 16
};

enum templates_argc_min {
	TEMPLATE_ARGCMIN_NOT_FOUND = 0,
	TEMPLATE_ARGCMIN_ROOT = 0,
	TEMPLATE_ARGCMIN_VARIABLE = 1,
	TEMPLATE_ARGCMIN_LOOP = 3,
	TEMPLATE_ARGCMIN_END = 0,
	TEMPLATE_ARGCMIN_SET_VAR = 2,
};

static void
templates_varlist_expand(void)
{
	if (variables.list == NULL)
	{
		variables.allocated = variables.ALLOC_CHUNK;
		variables.list = malloc(
				sizeof(struct variable) * variables.allocated);
	}
	else if (variables.length == (variables.allocated - 1))
	{
		variables.allocated += variables.ALLOC_CHUNK;
		variables.list = realloc(variables.list,
				sizeof(struct variable) * variables.allocated);
	}
}

static struct variable *
templates_varlist_get(const char *name)
{
	for (uint32_t i = 0; i < variables.length; ++i)
	{
		if (!strcmp(name, variables.list[i].name))
		{
			return &variables.list[i];
		}
	}

	return NULL;
}

static enum vartype
templates_dettype(const char *data)
{
	const uint32_t length = strlen(data);
	uint32_t tfloat = 0;
	uint32_t tint = 0;

	for (uint32_t i = 0; i < length; ++i)
	{
		if (i > 0 && data[i] == '.')
		{
			++tfloat;
		}
		else if ('0' <= data[i] && data[i] <= '9')
		{
			++tint;
		}
		else
		{
			return TEMPLATE_VARTYPE_STRING;
		}
	}

	if (tfloat == 1)
	{
		return TEMPLATE_VARTYPE_DOUBLE;
	}
	else if (tfloat > 1)
	{
		return TEMPLATE_VARTYPE_ERROR;
	}
	else
	{
		return TEMPLATE_VARTYPE_INT;
	}
}

void
templates_deinit(void)
{
	free(variables.list);
	variables.length = 0;
	variables.allocated = 0;
}

static enum templates_error_codes
templates_variable(FILE *stream,
		const uint32_t argc,
		const char **argv)
{
	if (argc < TEMPLATE_ARGCMIN_VARIABLE)
	{
		return TEMPLATE_ERROR_ARGCMIN_NSAT;
	}

	const struct variable *var = templates_varlist_get(argv[0]);
	if (var == NULL)
	{
		return TEMPLATE_ERROR_GETVAR_NOT_FOUND;
	}

	switch (var->type)
	{
	case TEMPLATE_VARTYPE_STRING:
		fprintf(stream, "%s", var->var.string);
		break;
	case TEMPLATE_VARTYPE_INT:
		fprintf(stream, "%d", var->var.num_int);
		break;
	case TEMPLATE_VARTYPE_DOUBLE:
		fprintf(stream, "%f", var->var.num_double);
		break;
	default:
		break;
	}

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_loop(FILE *stream,
		const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	(void) stream;
	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_end(FILE *stream,
		const uint32_t argc,
		const char **argv)
{
	(void) argc;
	(void) argv;
	(void) stream;
	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_set_var(FILE *stream,
		const uint32_t argc,
		const char **argv)
{
	(void) stream;
	if (argc < TEMPLATE_ARGCMIN_SET_VAR)
	{
		return TEMPLATE_ERROR_ARGCMIN_NSAT;
	}

	const char *name = argv[0];
	const char *data = argv[1];
	const enum vartype type = templates_dettype(data);

	if (type == TEMPLATE_VARTYPE_ERROR)
	{
		return TEMPLATE_ERROR_SETVAR_NO_TYPE;
	}

	templates_varlist_expand();
	struct variable * const var = &variables.list[variables.length++];
	strcpy(var->name, name);
	var->type = type;
	switch (var->type)
	{
	case TEMPLATE_VARTYPE_INT:
		var->var.num_int = atoi(data);
		break;
	case TEMPLATE_VARTYPE_DOUBLE:
		var->var.num_double = atof(data);
		break;
	case TEMPLATE_VARTYPE_STRING:
		strcpy(var->var.string, data);
		break;
	default:
		--variables.length;
		return TEMPLATE_ERROR_SETVAR_UNUSED_TYPE;
	}

	return TEMPLATE_ERROR_NONE;
}

struct templates_type_info {
	const char 			*keyword;
	const int32_t			max_params;
	enum templates_error_codes
		(* const func)(FILE *, const uint32_t, const char **);
};

static const struct templates_type_info templates_table[TEMPLATE_TOTAL] = {
	// enum templates_type	const char *	int32_t	function
	// 			keyword		max_p.
	[TEMPLATE_NOT_FOUND] 	= { NULL,	TEMPLATE_ARGCMIN_NOT_FOUND,	NULL },
	[TEMPLATE_ROOT] 	= { NULL,	TEMPLATE_ARGCMIN_ROOT,		NULL },
	[TEMPLATE_VARIABLE] 	= { "var",	TEMPLATE_ARGCMIN_VARIABLE,	templates_variable },
	[TEMPLATE_LOOP] 	= { "loop",	TEMPLATE_ARGCMIN_LOOP,		templates_loop },
	[TEMPLATE_END]		= { "end",	TEMPLATE_ARGCMIN_END,		templates_end },
	[TEMPLATE_SET_VAR] 	= { "set", 	TEMPLATE_ARGCMIN_SET_VAR,	templates_set_var },
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

enum templates_error_codes
templates(FILE *stream,
		const enum templates_type type,
		const uint32_t argc,
		const char ** const argv)
{
	enum templates_error_codes (* const func)
		(FILE *, const uint32_t, const char **)
		= templates_table[type].func;

	if (func != NULL)
	{
		return func(stream, argc, argv);
	}
	else
	{
		return TEMPLATE_ERROR_NO_FUNC;
	}
}

const char *
templates_error(const enum templates_error_codes code)
{
	static const char *error_codes_str[TEMPLATE_ERROR_TOTAL] = {
		[TEMPLATE_ERROR_NONE] = "none",
		[TEMPLATE_ERROR_ARGCMIN_NSAT] = "argc not satisfied",
		[TEMPLATE_ERROR_ARGV_ERROR] = "argv generic error",
		[TEMPLATE_ERROR_SETVAR_NO_TYPE] = "set var no type",
		[TEMPLATE_ERROR_SETVAR_UNUSED_TYPE] = "set var unused type",
		[TEMPLATE_ERROR_GETVAR_NOT_FOUND] = "get var not found",
		[TEMPLATE_ERROR_NO_FUNC] = "no function"
	};

	return error_codes_str[code];
}

