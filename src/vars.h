#ifndef VARS_H
#define VARS_H

#include <stdbool.h>
#include <stdint.h>

enum vars_error {
	VARS_ERROR_NONE = 0,
	VARS_ERROR_NOT_FOUND,
	VARS_ERROR_UNSUPPORTED_TYPE,
	VARS_ERROR_OUT_OF_RANGE,
	VARS_ERROR_SET_NOT_ALLOWED,

	VARS_ERROR_TOTAL
};

void vars_init(void);
void vars_deinit(void);
enum vars_error vars_set(const char *name,
		const char **values,
		const uint32_t length);
char *vars_get(const char *name,
		const char *offset,
		enum vars_error *error);
enum vars_error vars_loop_range(const char *name,
		const char *start_str,
		const char *end_str,
		bool *ended);
enum vars_error vars_loop_in(const char *name,
		const char *list,
		bool *ended);
enum vars_error vars_loop(const uint32_t argc,
		const char **argv,
		bool *ended);
void vars_loop_end(const uint32_t argc,
		const char **argv,
		const bool ended);

#endif // VARS_H

