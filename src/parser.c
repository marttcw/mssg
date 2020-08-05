#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	CHUNK_SIZE = 512,
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

static char 	parameter[PARSER_PARAM_MAX_CHARS] = { 0 };
static uint32_t	parameter_length = 0;

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
			//.parameters = { 0 },
			.parameters_length = { 0 },
			.char_begin = 0,
			.char_end = 0,
			.finding_type = true,
			.length = 0,
			.alloc_length = 0,
			.nodes = NULL,
			.parent = parser->current
		};

		parser->current = &parser->current->nodes[index];

		parser->state = PARSER_STATE_COND;
		break;
	case '{':
		parser->state = PARSER_STATE_VAR;
		break;
	default:
		parser->state = PARSER_STATE_COPY;
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
			// Appends to parameter
			const uint32_t index = parser->current->parameters_total++;
			strcpy(parser->current->parameters[index], parameter);
			parser->current->parameters_length[index] = parameter_length;
		}

		// Reset
		parameter[0] = '\0';
		parameter_length = 0;
		break;
	case '"':
		parser->state = PARSER_STATE_COND_QUOTE;
		break;
	default:
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
		printf("COND: %s\n", templates_type_to_str(parser->current->type));

		for (uint32_t i = 0; i < parser->current->parameters_total; ++i)
		{
			printf("\t%s\n", parser->current->parameters[i]);
		}

		parser->current->char_begin = parser->current_file_position;
		parser->current->char_end = parser->current_file_position;

		printf("Char pos: %ld\n", parser->current->char_begin);

		parser->current = parser->current->parent;
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
	default:
		putchar(character);
		break;
	}
}

static void
parser__read_char_from_var(struct parser *parser,
		const char character)
{
	switch (character)
	{
	case '}':
		parser->state = PARSER_STATE_COPY;
		break;
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
		.current = NULL
	};

	FILE *fp = fopen(filepath, "r");
	if (fp == NULL)
	{
		return (struct parser) { .error = PARSER_ERROR_FILE_NULL };
	}

	size_t read_size = CHUNK_SIZE;
	char chunk[CHUNK_SIZE + 1] = { 0 };
	parser.current = &parser.node;		// Point current to root node

	while (read_size == CHUNK_SIZE)
	{
		uint64_t file_position = ftell(fp);
		read_size = fread(chunk, sizeof(char), CHUNK_SIZE, fp);
		chunk[CHUNK_SIZE] = '\0';

		if (ferror(fp))
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

	putchar('\n');

	fclose(fp);

	return parser;
}

static void
parser__destroy_node(struct parser_node *node)
{
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
}

char *
parser_error_message(const struct parser *parser)
{
	return parser_error_string[parser->error];
}

