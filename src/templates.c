#include "templates.h"

#include <string.h>
#include <stdlib.h>

#include "hashmap.h"

enum vartype {
	TEMPLATE_VARTYPE_INT = 0,
	TEMPLATE_VARTYPE_DOUBLE,
	TEMPLATE_VARTYPE_STRING,
	TEMPLATE_VARTYPE_ERROR
};

struct variable {
	enum vartype	type;
	bool		settable;
	union {
		int32_t	num_int;
		double	num_double;
		char	string[256];
	} var;
};

struct block {
	FILE	*stream;
};

static void
templates__blocks_cleanup(void *data)
{
	struct block *block = data;
	if (block->stream != NULL)
	{
		fclose(block->stream);
	}
}

// In-file variables
//TODO: Move to parser?
static struct hashmap variables = HASHMAP_STRUCT_INIT(16, 8,
		struct variable, NULL);
static struct hashmap blocks = HASHMAP_STRUCT_INIT(16, 8,
		struct variable, templates__blocks_cleanup);

enum templates_argc_min {
	TEMPLATE_ARGCMIN_NOT_FOUND = 0,
	TEMPLATE_ARGCMIN_ROOT = 0,
	TEMPLATE_ARGCMIN_VARIABLE = 1,
	TEMPLATE_ARGCMIN_LOOP = 3,
	TEMPLATE_ARGCMIN_SET_VAR = 2,
	TEMPLATE_ARGCMIN_SET_BLOCK = 1,
	TEMPLATE_ARGCMIN_PUT_BLOCK = 1,
	TEMPLATE_ARGCMIN_BASE = 1,
	TEMPLATE_ARGCMIN_LINK = 1,
	TEMPLATE_ARGCMIN_END = 0,
};

void
file_append_file(FILE *out, FILE *in)
{
	enum {
		CHUNK_SIZE = 512
	};

	rewind(in);
	char chunk[CHUNK_SIZE] = { 0 };
	size_t read_size = CHUNK_SIZE;
	size_t write_size = 0;
	while (read_size == CHUNK_SIZE)
	{
		read_size = fread(chunk, sizeof(char), CHUNK_SIZE, in);
		write_size = fwrite(chunk, sizeof(char), read_size, out);

		if (ferror(in) || (read_size != write_size))
		{
			fprintf(stderr, "file_append_file: rw error.\n");
			break;
		}
	}
}

static struct variable *
templates_varlist_get(const char *name)
{
	return hashmap_get(&variables, name);
}

static struct block *
templates_block_get(const char *name)
{
	return hashmap_get(&blocks, name);
}

static enum vartype
templates_dettype(const char *data)
{
	const uint32_t length = strlen(data); uint32_t tfloat = 0; uint32_t tint = 0;

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

FILE *
templates__add_block(const char *name)
{
	// Find block first to override
	struct block *has_block = templates_block_get(name);
	if (has_block != NULL)
	{
		fclose(has_block->stream);
		has_block->stream = tmpfile();
		return has_block->stream;
	}

	struct block *block = hashmap_add(&blocks, name); 
	block->stream = tmpfile();
	return block->stream;
}

void
templates_deinit(void)
{
	hashmap_destroy(&variables);
	hashmap_destroy(&blocks);
}

static enum templates_error_codes
templates_variable(struct templates templates)
{
	FILE *out_stream = (*templates.generate_outside) ?
		templates.stream : *templates.indirect_stream;

	const struct variable *var = templates_varlist_get(templates.argv[0]);
	if (var == NULL)
	{
		return TEMPLATE_ERROR_GETVAR_NOT_FOUND;
	}

	switch (var->type)
	{
	case TEMPLATE_VARTYPE_STRING:
		fprintf(out_stream, "%s", var->var.string);
		break;
	case TEMPLATE_VARTYPE_INT:
		fprintf(out_stream, "%d", var->var.num_int);
		break;
	case TEMPLATE_VARTYPE_DOUBLE:
		fprintf(out_stream, "%f", var->var.num_double);
		break;
	default:
		break;
	}

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_loop__get_int(const char *data,
		int32_t *number)
{
	const enum vartype type = templates_dettype(data);
	switch (type)
	{
	case TEMPLATE_VARTYPE_INT:
		*number = atoi(data);
		break;
	case TEMPLATE_VARTYPE_STRING:
	{
		struct variable *var = templates_varlist_get(data);
		if (var == NULL)
		{
			return TEMPLATE_ERROR_GETVAR_NOT_FOUND;
		}
		*number = var->var.num_int;
	}	break;
	default:
		return TEMPLATE_ERROR_SETVAR_UNUSED_TYPE;
	}

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_loop(struct templates templates)
{
	const char *name = templates.argv[0];
	int32_t start = 0;
	int32_t end = 0;

	// Move to only init stage
	const char *start_data = templates.argv[1];
	const char *end_data = templates.argv[2];

	enum templates_error_codes error = TEMPLATE_ERROR_NONE;
	if ((error = templates_loop__get_int(start_data, &start))
			!= TEMPLATE_ERROR_NONE)
	{
		return error;
	}
	if ((error = templates_loop__get_int(end_data, &end))
			!= TEMPLATE_ERROR_NONE)
	{
		return error;
	}

	const int32_t diff = (start <= end) ? 1 : -1;
	//printf("templates_loop: %s %d %d\n", name, start, end);

	struct variable *var = templates_varlist_get(name);
	if (var == NULL)
	{	// First execution of loop
		var = hashmap_add(&variables, name);
		var->type = TEMPLATE_VARTYPE_INT;
		var->var.num_int = start;
		var->settable = false;

		// Add into start/end pool
	}
	else
	{
		var->var.num_int += diff;
	}

	*templates.tscondgen = 1;
	if (var->var.num_int == end)
	{
		// TODO: Remove variable
		*templates.tscondgen = 0;
	}

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_set_var(struct templates templates)
{
	const char *name = templates.argv[0];
	const char *data = templates.argv[1];
	const enum vartype type = templates_dettype(data);

	if (type == TEMPLATE_VARTYPE_ERROR)
	{
		return TEMPLATE_ERROR_SETVAR_NO_TYPE;
	}

	// Find var first to override
	struct variable *var = templates_varlist_get(name);
	if (var == NULL)
	{	// Make a new variable
		var = hashmap_add(&variables, name);
		var->settable = true;
	}

	if (!var->settable)
	{
		return TEMPLATE_ERROR_SETVAR_DISALLOWED;
	}

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
		return TEMPLATE_ERROR_SETVAR_UNUSED_TYPE;
	}

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_set_block(struct templates templates)
{
	*templates.generate_outside = false;
	*templates.indirect_stream = templates__add_block(templates.argv[0]);

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_put_block(struct templates templates)
{
	FILE *file = templates_block_get(templates.argv[0])->stream;
	if (file)
	{
		file_append_file(templates.stream, file);
	}
	else
	{
		fprintf(stderr, "ERROR: Cannot get file '%s'!\n",
				templates.argv[0]);
	}
	return TEMPLATE_ERROR_NONE;
}

static bool
link_is_dir(const char *link)
{
	const uint32_t len = strlen(link);
	if (link[len-1] == '/')
	{
		return true;
	}

	for (int32_t i = (len-1); i >= 0; --i)
	{
		if (link[i] == '.')
		{
			return false;
		}
		else if (link[i] == '/')
		{
			return true;
		}
	}

	return true;
}

static uint32_t
level_from_base(const char *base_dir,
		const char *cur_file)
{
	const uint32_t len_base_dir = strlen(base_dir);
	uint32_t base_dir_count = 0;

	for (uint32_t i = 0; i < len_base_dir; ++i)
	{
		if (base_dir[i] == '/')
		{
			++base_dir_count;
		}
	}

	if (base_dir[len_base_dir-1] != '/')
	{
		++base_dir_count;
	}

	const uint32_t len_cur_file = strlen(cur_file);
	uint32_t cur_file_count = 0;
	for (uint32_t i = 0; i < len_cur_file; ++i)
	{
		if (cur_file[i] == '/')
		{
			++cur_file_count;
		}
	}

	return cur_file_count - base_dir_count;
}

static enum templates_error_codes
templates_link(struct templates templates)
{
#if 0
	printf("templates:\n\tbase_dir: %s\n\tmain_file: %s\n\t"
			"dest_dir: %s\n\tlink: %s\n",
			templates.base_dir, templates.main_file,
			templates.dest_dir, templates.argv[0]);
#endif

	const char *link = templates.argv[0];
	char expand[256] = { 0 };
	char prefix[256] = { 0 };
	char final[512] = { 0 };
	const uint32_t len = strlen(link);

	// Expand with index.html (local only)
	if (link_is_dir(link))
	{
		// TODO if local testing
		sprintf(expand, "%s%sindex.html",
				link,
				(link[len-1] == '/') ? "" : "/");
	}
	else
	{
		sprintf(expand, "%s", link);
	}

	if (link[0] == '/')
	{
		// Prefix with ../ (local only)
		const uint32_t levels = level_from_base(templates.dest_dir,
				templates.main_file);

		for (uint32_t i = 0; i < levels; ++i)
		{
			strcat(prefix, "../");
		}

		sprintf(final, "%s%s", prefix, expand + 1);
	}
	else
	{
		sprintf(final, "%s", expand);
	}

#if 0
	printf("\t\tfinal: %s\n", final);
#endif
	if (templates.argc >= 2)
	{
		fprintf(templates.stream, "<a href=\"%s\">%s</a>",
				final, templates.argv[1]);
	}
	else
	{
		fprintf(templates.stream, "%s", final);
	}

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_end(struct templates templates)
{
	*templates.generate_outside = true;

#if 0
	switch (templates.parent_type)
	{
	default:
		break;
	}
#endif

	*templates.indirect_stream = NULL;

	return TEMPLATE_ERROR_NONE;
}

struct templates_type_info {
	const char 			*keyword;
	const uint32_t			max_params;
	enum templates_error_codes (* const func)(struct templates);
};

static const struct templates_type_info templates_table[TEMPLATE_TOTAL] = {
	// enum templates_type	const char *	int32_t	function
	// 			keyword		max_p.
	[TEMPLATE_NOT_FOUND] 	= { NULL,	TEMPLATE_ARGCMIN_NOT_FOUND,	NULL },
	[TEMPLATE_ROOT] 	= { NULL,	TEMPLATE_ARGCMIN_ROOT,		NULL },
	[TEMPLATE_VARIABLE] 	= { "var",	TEMPLATE_ARGCMIN_VARIABLE,	templates_variable },
	[TEMPLATE_LOOP] 	= { "loop",	TEMPLATE_ARGCMIN_LOOP,		templates_loop },
	[TEMPLATE_SET_VAR] 	= { "set", 	TEMPLATE_ARGCMIN_SET_VAR,	templates_set_var },
	[TEMPLATE_SET_BLOCK]	= { "setblock",	TEMPLATE_ARGCMIN_SET_BLOCK,	templates_set_block },
	[TEMPLATE_PUT_BLOCK]	= { "putblock",	TEMPLATE_ARGCMIN_PUT_BLOCK,	templates_put_block },
	[TEMPLATE_BASE]		= { "base",	TEMPLATE_ARGCMIN_BASE,		NULL },
	[TEMPLATE_LINK]		= { "link",	TEMPLATE_ARGCMIN_LINK,		templates_link },
	[TEMPLATE_END]		= { "end",	TEMPLATE_ARGCMIN_END,		templates_end },
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
templates(struct templates templates)
{
	const struct templates_type_info row = templates_table[templates.type];

	if (row.func != NULL)
	{

		if (templates.argc < row.max_params)
		{
			return TEMPLATE_ERROR_ARGCMIN_NSAT;
		}
		else
		{
			return row.func(templates);
		}
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
		[TEMPLATE_ERROR_NO_FUNC] = "no function",
		[TEMPLATE_ERROR_SETVAR_DISALLOWED] = "not settable",
	};

	return error_codes_str[code];
}

