#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "templates.h"

enum {
	CHUNK_SIZE = 512,
	PARAM_CHUNK = 256,
	ALLOC_STR = 64,
	ALLOC_STEP = 4
};

struct parser_funcs {
	void (*read_char)(struct parser *parser, const char character);
};

struct parser_type_info {
	const char 	*keyword;
	const int32_t	max_params;
};

static char *parser_error_string[PARSER_ERROR_TOTAL] = {
	[PARSER_ERROR_NONE] 		= "No error has occured",
	[PARSER_ERROR_FILE_NULL] 	= "Cannot read file: File does not exists or permission not granted to read",
	[PARSER_ERROR_FILE_ERROR]	= "Error during file reading occured"
};

static char 	*parameter = NULL;
static uint32_t	parameter_length = 0;

void
parser_deinit(void)
{
	if (parameter != NULL)
	{
		free(parameter);
	}

	parameter_length = 0;
}

static void
parser__read_char_copy(struct parser *parser,
		const char character)
{
	switch (character)
	{
	case '{':
		parser->state = PARSER_STATE_FROM_COPY;
		break;
	default:
		// TODO: Copy over
		break;
	}
}

static void
parser__read_char_from_copy(struct parser *parser,
		const char character)
{
	switch (character)
	{
	case '%':
	case '{':
		// Allocate memory if not there
		if (parser->current->nodes == NULL ||
				parser->current->alloc_length == 0)
		{
			parser->current->alloc_length = ALLOC_STEP;

			parser->current->nodes = malloc(
					sizeof(struct parser_node) *
						parser->current->alloc_length);
		}

		const uint32_t index = parser->current->length++;

		// Allocate more memory if required
		if (parser->current->alloc_length <= parser->current->length)
		{
			parser->current->alloc_length += ALLOC_STEP;

			parser->current->nodes = realloc(
					parser->current->nodes,
					sizeof(struct parser_node) *
						parser->current->alloc_length);
		}

		// Initialize node
		parser->current->nodes[index] = (struct parser_node) {
			.type = TEMPLATE_NOT_FOUND,
			.argc = 0,
			.argl = NULL,
			.argv = NULL,
			.arga = 0,
			.char_begin = 0,
			.char_end = 0,
			.finding_type = true,
			.length = 0,
			.alloc_length = 0,
			.nodes = NULL,
			.parent = parser->current
		};

		parser->current = &parser->current->nodes[index];
		parser->current->argl = malloc(sizeof(uint32_t) * ALLOC_STEP);
		parser->current->argv = malloc(sizeof(char *) * ALLOC_STEP);
		for (uint32_t i = 0; i < ALLOC_STEP; ++i)
		{
			parser->current->argv[i] = calloc(sizeof(char),
					ALLOC_STR);
		}
		parser->current->arga = ALLOC_STEP;
		parser->current->char_begin = parser->current_file_position-1;
		break;
	default:
		parser->state = PARSER_STATE_COPY;
		return;
	}

	switch (character)
	{
	case '%':
		parser->state = PARSER_STATE_COND;
		break;
	case '{':
		parser->current->type = TEMPLATE_VARIABLE;
		parser->current->argc = 0;
		parser->current->argl[0] = 0;
		parser->current->finding_type = false;

		parser->state = PARSER_STATE_VAR;
		break;
	default:
		break;
	}
}

static void
parser__read_char_cond(struct parser *parser,
		const char character)
{

	switch (character)
	{
	case '%':
		parser->state = PARSER_STATE_FROM_COND;
		break;
	case ' ':
		if (parameter_length == 0)
		{
			break;
		}

		parameter[parameter_length] = '\0';

		if (parser->current->finding_type)
		{
			// Match with the type
			parser->current->type = templates_str_to_type(parameter);

			if (parser->current->type == TEMPLATE_NOT_FOUND)
			{
				fprintf(stderr, "ERROR: '%s' not a recognised command!\n",
						parameter);
			}

			parser->current->finding_type = false;
		}
		else
		{
			const uint32_t index = parser->current->argc++;
			// Appends to parameter
			if (parser->current->argv == NULL)
			{
				parser->current->argv = malloc(
						sizeof(char) *
						(parameter_length + 1));
			}
			strcpy(parser->current->argv[index], parameter);
			parser->current->argl[index] = parameter_length;
		}

		// Reset
		parameter[0] = '\0';
		parameter_length = 0;
		break;
	case '"':
		parser->state = PARSER_STATE_COND_QUOTE;
		break;
	default:
		if (parameter == NULL)
		{
			parameter = malloc(sizeof(char) * PARAM_CHUNK);
		}

		parameter[parameter_length++] = character;
		break;
	}
}

static void
parser__read_char_cond_quote(struct parser *parser,
		const char character)
{
	switch (character)
	{
	case '"':
		parser->state = PARSER_STATE_COND;
		break;
	default:
		parameter[parameter_length++] = character;
		break;
	}
}

static void
parser__read_char_from_cond(struct parser *parser,
		const char character)
{
	switch (character)
	{
	case '}':
		parser->current->char_end = parser->current_file_position;

		const enum templates_type ttype = parser->current->type;

		switch (ttype)
		{
		case TEMPLATE_LOOP:
			break;
		case TEMPLATE_END:
			parser->current = parser->current->parent->parent;
			break;
		default:
			parser->current = parser->current->parent;
		}

		parser->state = PARSER_STATE_COPY;
		break;
	default:
		parser->state = PARSER_STATE_COND;
		break;
	}
}

static void
parser__read_char_var(struct parser *parser,
		const char character)
{
	switch (character)
	{
	case '}':
		parser->state = PARSER_STATE_FROM_VAR;
		break;
	case ' ':
		break;
	default:
	{
		const uint32_t index = parser->current->argl[0]++;
		parser->current->argv[0][index] = character;
	}	break;
	}
}

static void
parser__read_char_from_var(struct parser *parser,
		const char character)
{
	switch (character)
	{
	case '}':
	{
		const uint32_t index = parser->current->argl[0];
		parser->current->argv[0][index] = '\0';
		parser->current->argc = 1;
		parser->current->char_end = parser->current_file_position;

		parser->current = parser->current->parent;
		parser->state = PARSER_STATE_COPY;
	}	break;
	default:
		parser->state = PARSER_STATE_VAR;
		break;
	}
}

/*
 * Maps states with its relative functions
 */
static const struct parser_funcs parser_funcs[PARSER_STATE_TOTAL] = {
	[PARSER_STATE_COPY] = {
		.read_char = parser__read_char_copy
	},

	[PARSER_STATE_FROM_COPY] = {
		.read_char = parser__read_char_from_copy
	},

	[PARSER_STATE_COND] = {
		.read_char = parser__read_char_cond
	},

	[PARSER_STATE_COND_QUOTE] = {
		.read_char = parser__read_char_cond_quote
	},

	[PARSER_STATE_FROM_COND] = {
		.read_char = parser__read_char_from_cond
	},

	[PARSER_STATE_VAR] = {
		.read_char = parser__read_char_var
	},

	[PARSER_STATE_FROM_VAR]	= {
		.read_char = parser__read_char_from_var
	}
};

static inline void
parser__read_char(struct parser *parser,
		const char character)
{
	parser_funcs[parser->state].read_char(parser, character);
}

struct parser
parser_create(const char *filepath)
{
	struct parser parser = {
		.error = PARSER_ERROR_NONE,
		.state = PARSER_STATE_COPY,
		.node = {
			.type = TEMPLATE_ROOT,
			.char_begin = 0,
			.char_end = 0,
			.length = 0,
			.nodes = NULL
		},
		.current = NULL,
		.fp = NULL
	};

	parser.fp = fopen(filepath, "r");
	if (parser.fp == NULL)
	{
		return (struct parser) { .error = PARSER_ERROR_FILE_NULL };
	}

	size_t read_size = CHUNK_SIZE;
	char chunk[CHUNK_SIZE + 1] = { 0 };
	parser.current = &parser.node;		// Point current to root node

	while (read_size == CHUNK_SIZE)
	{
		uint64_t file_position = ftell(parser.fp);
		read_size = fread(chunk, sizeof(char), CHUNK_SIZE, parser.fp);
		chunk[CHUNK_SIZE] = '\0';

		if (ferror(parser.fp))
		{
			parser.error = PARSER_ERROR_FILE_ERROR;
			break;
		}

		for (uint32_t i = 0; i < read_size; ++i, ++file_position)
		{
			// Parse the character
			parser.current_file_position = file_position;
			parser__read_char(&parser, chunk[i]);
		}
	}

	return parser;
}

static void
parser__destroy_node(struct parser_node *node)
{
	free(node->argl);
	for (uint32_t i = 0; i < node->arga; ++i)
	{
		free(node->argv[i]);
	}
	free(node->argv);

	for (uint32_t i = 0; i < node->length; ++i)
	{
		parser__destroy_node(&node->nodes[i]);
	}

	if (node->nodes != NULL)
	{
		free(node->nodes);
		node->nodes = NULL;
		node->alloc_length = 0;
		node->length = 0;
	}
}

void
parser_destroy(struct parser *parser)
{
	parser__destroy_node(&parser->node);
	fclose(parser->fp);
}

char *
parser_error_message(const struct parser *parser)
{
	return parser_error_string[parser->error];
}

void
parser__node_print(const struct parser_node *node, const uint32_t level)
{
	for (uint32_t i = 0; i < level; ++i)
	{
		printf("    ");
	}

	printf("type: %s | params: %d | char: %ld => %ld |"
			" finding_type: %d | nodes: %d | ", 
			templates_type_to_str(node->type),
			node->argc, node->char_begin,
			node->char_end, node->finding_type, node->length);

	for (uint32_t i = 0; i < node->argc; ++i)
	{
		printf("%s ", node->argv[i]);
	}

	putchar('\n');

	for (uint32_t i = 0; i < node->length; ++i)
	{
		parser__node_print(&node->nodes[i], level + 1);
	}
}

void
parser_print(const struct parser *parser)
{
	parser__node_print(&parser->node, 0);
}

uint64_t
parser__generate_node(const struct parser_node *node,
		FILE *stream,
		FILE *fp)
{
	static uint64_t prev_pos = 0;
	const uint64_t cur_pos = node->char_begin;
	const uint64_t total_len_read = cur_pos - prev_pos - 1;
	uint64_t rem_len_read = total_len_read;
	char chunk[CHUNK_SIZE] = { 0 };

	// Copy over non-nodes pos
	fseek(fp, prev_pos + 1, SEEK_SET);

	while (rem_len_read)
	{
		uint32_t chunk_read = CHUNK_SIZE;
		if (rem_len_read < CHUNK_SIZE)
		{
			chunk_read = rem_len_read;
			rem_len_read = 0;
		}
		else
		{
			rem_len_read -= CHUNK_SIZE;
		}
		size_t read_size = fread(chunk, sizeof(char), chunk_read, fp);
		chunk[chunk_read] = '\0';

		if (ferror(fp) || (read_size != chunk_read) ||
				(chunk[0] == '\n' && chunk[1] == '\0'))
		{
			break;
		}

		fprintf(stream, "%s", chunk);
	}

	const enum templates_error_codes error = 
		templates(stream, node->type, node->argc,
			(const char ** const) node->argv);

	if (error != TEMPLATE_ERROR_NONE &&
			error != TEMPLATE_ERROR_NO_FUNC)
	{
		printf("%s\n", templates_error(error));
	}

	prev_pos = node->char_end;

	for (uint32_t i = 0; i < node->length; ++i)
	{
		prev_pos = parser__generate_node(&node->nodes[i], stream, fp);
	}

	return prev_pos;
}

void
parser_generate(const struct parser *parser,
		FILE *stream)
{
	rewind(parser->fp);
	const uint64_t final_pos = parser__generate_node(&parser->node, stream, parser->fp);

	// Read final part after final template
	char chunk[CHUNK_SIZE] = { 0 };

	// Copy over non-nodes pos
	fseek(parser->fp, final_pos + 1, SEEK_SET);
	size_t read_size = CHUNK_SIZE;

	while (read_size == CHUNK_SIZE)
	{
		read_size = fread(chunk, sizeof(char), CHUNK_SIZE, parser->fp);
		chunk[read_size] = '\0';

		if (ferror(parser->fp))
		{
			break;
		}

		fprintf(stream, "%s", chunk);
	}
	fprintf(stream, "\n");
}

