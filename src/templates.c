#include "templates.h"

#include <string.h>
#include <stdlib.h>

#include "hashmap.h"
#include "copy.h"
#include "vars.h"

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
static struct hashmap blocks = { 0 };

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

static struct block *
templates_block_get(const char *name)
{
	return hashmap_get(&blocks, name);
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
templates_init(void)
{
	vars_init();
	hashmap_create(&blocks, 16, 8, sizeof(struct block),
			templates__blocks_cleanup,
			NULL);
}

void
templates_deinit(void)
{
	vars_deinit();
	hashmap_destroy(&blocks);
}

static enum templates_error_codes vars_temp_error_map[VARS_ERROR_TOTAL] = {
	[VARS_ERROR_NONE] = TEMPLATE_ERROR_NONE,
	[VARS_ERROR_NOT_FOUND] = TEMPLATE_ERROR_GETVAR_NOT_FOUND,
	[VARS_ERROR_UNSUPPORTED_TYPE] = TEMPLATE_ERROR_GETVAR_NOT_FOUND,
	[VARS_ERROR_OUT_OF_RANGE] = TEMPLATE_ERROR_GETVAR_INDEX_OUT_OF_RANGE,
	[VARS_ERROR_SET_NOT_ALLOWED] = TEMPLATE_ERROR_SETVAR_DISALLOWED,
};

static enum templates_error_codes
templates_variable(struct templates templates)
{
	enum vars_error error = VARS_ERROR_NONE;
	char *value = (templates.argc == 1) ?
		vars_get(templates.argv[0], NULL, &error) :
		vars_get(templates.argv[0], templates.argv[1], &error);

	if ((value == NULL) || error)
	{
		return vars_temp_error_map[error];
	}
	else
	{
		FILE *out_stream = (*templates.generate_outside) ?
			templates.stream :
			*templates.indirect_stream;

		fprintf(out_stream, "%s", value);
		return TEMPLATE_ERROR_NONE;
	}
}

static enum templates_error_codes
templates_loop(struct templates templates)
{
	bool ended = false;

	enum vars_error error = vars_loop(templates.argc,
			(const char **) templates.argv,
			&ended);

	if (error != VARS_ERROR_NONE)
	{
		return vars_temp_error_map[error];
	}

	*templates.tscondgen = (ended) ? 0 : 1;
	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_set_var(struct templates templates)
{
	return vars_temp_error_map[vars_set(templates.argv[0],
			(const char **) templates.argv + 1,
			templates.argc - 1)];
}

static enum templates_error_codes
templates_set_block(struct templates templates)
{
	*templates.generate_outside = false;
	*templates.indirect_stream = templates__add_block(templates.argv[0]);

	return TEMPLATE_ERROR_NONE;
}

// TODO
static enum templates_error_codes
templates_set_dir(struct templates templates)
{
	(void) templates;
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

	switch (templates.parent_type)
	{
	case TEMPLATE_LOOP:
		vars_loop_end(templates.parent_argc,
				templates.parent_argv,
				(*templates.tscondgen == 0));
		break;
	default:
		break;
	}

	*templates.indirect_stream = NULL;

	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_copy(struct templates templates)
{
	(void) templates;
	return TEMPLATE_ERROR_NONE;
}

static enum templates_error_codes
templates_copy_ignore(struct templates templates)
{
	for (uint32_t i = 0; i < templates.argc; ++i)
	{
		copy_ignore(templates.argv[i], COPY_FTYPE_FILENAME);
	}
	return TEMPLATE_ERROR_NONE;
}

static const struct templates_type_info {
	const char 			*keyword;
	const uint32_t			min_params;
	enum templates_error_codes (* const func)(struct templates);
} templates_table[TEMPLATE_TOTAL] = {
	// enum templates_type	keyword		min_params	func
	[TEMPLATE_NOT_FOUND] 	= { NULL,	0,	NULL },
	[TEMPLATE_ROOT] 	= { NULL,	0,	NULL },
	[TEMPLATE_VARIABLE] 	= { "var",	1,	templates_variable },
	[TEMPLATE_LOOP] 	= { "loop",	2,	templates_loop },
	[TEMPLATE_SET_VAR] 	= { "set", 	2,	templates_set_var },
	[TEMPLATE_SET_BLOCK]	= { "setblock",	1,	templates_set_block },
	[TEMPLATE_PUT_BLOCK]	= { "putblock",	1,	templates_put_block },
	[TEMPLATE_BASE]		= { "base",	1,	NULL },
	[TEMPLATE_LINK]		= { "link",	1,	templates_link },
	[TEMPLATE_END]		= { "end",	0,	templates_end },
	[TEMPLATE_COPY]		= { "copy",	1,	templates_copy },
	[TEMPLATE_COPY_IGNORE]	= { "copy_ignore", 1,	templates_copy_ignore },
	[TEMPLATE_SET_DIR]	= { "setdir",	2,	templates_set_dir },
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

		if (templates.argc < row.min_params)
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
		[TEMPLATE_ERROR_GETVAR_INDEX_OUT_OF_RANGE] = "get var out of range",
		[TEMPLATE_ERROR_NO_FUNC] = "no function",
		[TEMPLATE_ERROR_SETVAR_DISALLOWED] = "not settable",
	};

	return error_codes_str[code];
}

