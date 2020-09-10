#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

enum templates_type {
	TEMPLATE_NOT_FOUND = 0,
	TEMPLATE_ROOT,
	TEMPLATE_VARIABLE,	// Maybe rename to: GET_VAR?
	TEMPLATE_LOOP,		// TODO
	TEMPLATE_SET_VAR,
	TEMPLATE_SET_BLOCK,
	TEMPLATE_SET_DIR,
	TEMPLATE_PUT_BLOCK,
	TEMPLATE_BASE,
	TEMPLATE_LINK,
	TEMPLATE_END,

	// Config file only
	TEMPLATE_COPY,
	TEMPLATE_COPY_IGNORE,

	TEMPLATE_TOTAL
};

enum templates_error_codes {
	TEMPLATE_ERROR_NONE = 0,
	TEMPLATE_ERROR_ARGCMIN_NSAT,	// Minimum argc not satisfied
	TEMPLATE_ERROR_ARGV_ERROR,	// Generic argv error
	TEMPLATE_ERROR_SETVAR_NO_TYPE,	// Var setter got no type out
	TEMPLATE_ERROR_SETVAR_UNUSED_TYPE,	// Var setter not using the type
	TEMPLATE_ERROR_GETVAR_NOT_FOUND,	// Var not found
	TEMPLATE_ERROR_GETVAR_INDEX_OUT_OF_RANGE,	// Var out of range
	TEMPLATE_ERROR_NO_FUNC,		// No function
	TEMPLATE_ERROR_SETVAR_DISALLOWED,	// Var setter not allowed

	TEMPLATE_ERROR_TOTAL
};

struct templates {
	FILE 			*stream;
	enum templates_type 	type;
	uint32_t 		argc;
	char 			**argv;
	bool 			*generate_outside;
	FILE 			**indirect_stream;
	enum templates_type 	parent_type;
	uint32_t 		parent_argc;
	const char 		**parent_argv;
	const char		*base_dir;
	const char		*cur_file;
	const char		*main_file;
	const char		*dest_dir;
	uint32_t		*tscondgen; // Type-Specific generic condition
};

void templates_init(void);
void templates_deinit(void);
enum templates_error_codes templates(struct templates templates);
enum templates_type templates_str_to_type(const char *keyword);
const char *templates_type_to_str(const enum templates_type type);
const char *templates_error(const enum templates_error_codes code);

#endif // TEMPLATES_H

