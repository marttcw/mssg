#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#include "templates.h"
#include "generic_list.h"

enum config_state {
	CONFIG_STATE_ARG = 0,
	CONFIG_STATE_ARG_QUOTE,

	CONFIG_STATE_MAX
};

struct config_line {
	enum templates_type	type;
	uint32_t		argc;
	uint32_t		*argl;
	char			**argv;
	uint32_t		arga;
};

struct config {
	enum config_state	state;
	struct generic_list	list;

	bool			finding_type;
	char			*param;
	uint32_t		param_length;
};

void config_create(struct config *config);
void config_destroy(struct config *config);
void config_parser(struct config *config,
		const char *filepath);
void config_print(struct config *config);
void config_template(struct config *config,
		const char *base_dir,
		const char *src_dir);

#endif // CONFIG_H

