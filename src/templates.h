#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <stdint.h>

enum templates_type {
	TEMPLATE_NOT_FOUND = 0,
	TEMPLATE_ROOT,
	TEMPLATE_VARIABLE,
	TEMPLATE_LOOP,
	TEMPLATE_END,
	TEMPLATE_SETTER_STR,
	TEMPLATE_SETTER_NUM,

	TEMPLATE_TOTAL
};

int32_t templates(const uint32_t argc,
		const char **argv);
enum templates_type templates_str_to_type(const char *keyword);
const char *templates_type_to_str(const enum templates_type type);

#endif // TEMPLATES_H

