#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <inttypes.h>

#include "templates.h"

enum {
	PARSER_PARAM_MAX_TOTAL 	= 3,
	PARSER_PARAM_MAX_CHARS 	= 64
};

enum parser_error {
	PARSER_ERROR_NONE = 0,
	PARSER_ERROR_FILE_NULL,
	PARSER_ERROR_FILE_ERROR,

	PARSER_ERROR_TOTAL
};

enum parser_state {
	PARSER_STATE_COPY = 0,
	PARSER_STATE_FROM_COPY,
	PARSER_STATE_COND,
	PARSER_STATE_COND_QUOTE,
	PARSER_STATE_FROM_COND,
	PARSER_STATE_VAR,
	PARSER_STATE_FROM_VAR,

	PARSER_STATE_TOTAL
};

struct parser_node {
	enum templates_type	type;
	char			parameters[PARSER_PARAM_MAX_TOTAL][PARSER_PARAM_MAX_CHARS];
	uint32_t		parameters_length[PARSER_PARAM_MAX_TOTAL];
	uint32_t		parameters_total;
	uint64_t		char_begin;
	uint64_t		char_end;
	bool			finding_type;

	uint32_t		length;
	uint32_t		alloc_length;
	struct parser_node	*nodes;
	struct parser_node	*parent;
};

struct parser {
	enum parser_error	error;
	enum parser_state	state;
	uint64_t		current_file_position;
	struct parser_node	node;
	struct parser_node	*current;
};

struct parser parser_create(const char *filepath);
void parser_destroy(struct parser *parser);
char *parser_error_message(const struct parser *parser);
void parser_print(const struct parser *parser);

#endif // PARSER_H

