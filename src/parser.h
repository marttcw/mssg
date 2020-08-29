#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <inttypes.h>

#include "templates.h"
#include "files.h"

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
	uint32_t		argc;
	uint32_t		*argl;
	char			**argv;
	uint32_t		arga;
	uint64_t		char_begin;
	uint64_t		char_end;
	uint32_t		line;
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
	FILE			*fp;
	uint32_t		line;
	char			filepath[256];

	bool			has_base;
	char			base_filepath[256];
	struct parser		*base;
};

void parser_deinit(void);
enum parser_error parser_create(struct parser *parser,
		const char *filepath,
		const char *root_filepath,
		struct files *files);
void parser_destroy(struct parser *parser);
char *parser_error_message(const struct parser *parser);
void parser_print(const struct parser *parser);
void parser_generate(const struct parser *parser, FILE *stream);

#endif // PARSER_H

