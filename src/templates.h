#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <stdio.h>
#include <stdint.h>

enum templates_type {
	TEMPLATE_NOT_FOUND = 0,
	TEMPLATE_ROOT,
	TEMPLATE_VARIABLE,
	TEMPLATE_LOOP,
	TEMPLATE_END,
	TEMPLATE_SET_VAR,

	TEMPLATE_TOTAL
};

enum templates_error_codes {
	TEMPLATE_ERROR_NONE = 0,
	TEMPLATE_ERROR_ARGCMIN_NSAT,	// Minimum argc not satisfied
	TEMPLATE_ERROR_ARGV_ERROR,	// Generic argv error
	TEMPLATE_ERROR_SETVAR_NO_TYPE,	// Var setter got no type out
	TEMPLATE_ERROR_SETVAR_UNUSED_TYPE,	// Var setter not using the type
	TEMPLATE_ERROR_GETVAR_NOT_FOUND,	// Var not found
	TEMPLATE_ERROR_NO_FUNC,		// No function

	TEMPLATE_ERROR_TOTAL
};

void templates_deinit(void);
enum templates_error_codes templates(FILE *stream,
		const enum templates_type type,
		const uint32_t argc,
		const char ** const argv);
enum templates_type templates_str_to_type(const char *keyword);
const char *templates_type_to_str(const enum templates_type type);
const char *templates_error(const enum templates_error_codes code);

#endif // TEMPLATES_H

