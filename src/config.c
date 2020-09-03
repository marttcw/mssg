#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	CHUNK_SIZE = 512,
	ARG_ALLOC_STEP = 4,
	ARG_ALLOC_STR = 512
};

static void
config__line_cleanup(void *data)
{
	struct config_line *line = data;
	free(line->argl);
	for (uint32_t i = 0; i < line->arga; ++i)
	{
		free(line->argv[i]);
	}
	free(line->argv);
	line->arga = 0;
}

void
config_create(struct config *config)
{
	config->state = CONFIG_STATE_ARG;
	config->finding_type = true;
	config->param = calloc(sizeof(char), 512);
	config->param_length = 0;
	generic_list_create(&config->list, 16, sizeof(struct config_line),
			config__line_cleanup);
}

void
config_destroy(struct config *config)
{
	if (config->param != NULL)
	{
		free(config->param);
	}
	generic_list_destroy(&config->list);
}

static void
config__add_param(struct config *config)
{
	struct config_line *line = generic_list_get_last(&config->list);
	if (line->arga == 0)
	{	// New line
		line->arga = ARG_ALLOC_STEP;
		line->argl = calloc(sizeof(uint32_t),
				line->arga);
		line->argv = calloc(sizeof(char *),
				line->arga);
		for (uint32_t i = 0; i < line->arga; ++i)
		{
			line->argv[i] = calloc(sizeof(char),
					ARG_ALLOC_STR);
		}
	}
	else if (line->argc >= (line->arga - 1))
	{	// Reallocate line
		const uint32_t i_start = line->arga;
		line->arga += ARG_ALLOC_STEP;
		line->argl = realloc(line->argl,
				sizeof(uint32_t) * line->arga);
		line->argv = realloc(line->argv,
				sizeof(char *) * line->arga);
		for (uint32_t i = i_start; i < line->arga; ++i)
		{
			line->argv[i] = calloc(sizeof(char),
					ARG_ALLOC_STR);
		}
	}

	config->param[config->param_length] = '\0';

	const uint32_t index = line->argc++;
	line->argl[index] = config->param_length;
	strcpy(line->argv[index], config->param);
}

static inline void
config__reset_param(struct config *config)
{
	config->param_length = 0;
	config->param[0] = '\0';
}

static void
config__parser_arg(struct config *config,
		const char ch)
{
	switch (ch)
	{
	case '\n':
	{
		if (config->param_length > 0)
		{
			config__add_param(config);
		}
		config__reset_param(config);
		config->finding_type = true;
	}	break;
	case '"':
		config->state = CONFIG_STATE_ARG_QUOTE;
		break;
	case ' ':
	{
		if (config->param_length == 0)
		{
			break;
		}

		if (config->finding_type)
		{
			struct config_line *line = generic_list_add(&config->list);
			line->type = templates_str_to_type(config->param);
			line->argc = 0;
			line->argl = NULL;
			line->argv = NULL;
			line->arga = 0;
			config->finding_type = false;
		}
		else
		{
			config__add_param(config);
		}
		config__reset_param(config);
	}	break;
	default:
		config->param[config->param_length++] = ch;
		break;
	}
}

static void
config__parser_arg_quote(struct config *config,
		const char ch)
{
	switch (ch)
	{
	case '"':
		config->state = CONFIG_STATE_ARG;
		break;
	default:
		config->param[config->param_length++] = ch;
		break;
	}
}

static void
config__parser_read(struct config *config,
		const char ch)
{
	static void (* const states_func[CONFIG_STATE_MAX])
		(struct config *, const char) = {
			[CONFIG_STATE_ARG] = config__parser_arg,
			[CONFIG_STATE_ARG_QUOTE] = config__parser_arg_quote,
	};
	states_func[config->state](config, ch);
}

void
config_parser(struct config *config,
		const char *filepath)
{
	FILE *fp = fopen(filepath, "r");
	char chunk[CHUNK_SIZE] = { 0 };

	while (fgets(chunk, CHUNK_SIZE, fp) != NULL)
	{
		uint32_t len = strlen(chunk);
		for (uint32_t i = 0; i < len; ++i)
		{
			config__parser_read(config, chunk[i]);
		}
	}

	fclose(fp);
}

void
config_print(struct config *config)
{
	printf("config file gives:\n");
	for (uint32_t i = 0; i < config->list.length; ++i)
	{
		const struct config_line *line = config->list.list[i];
		printf("\ttype: %s | argc: %d | ",
				templates_type_to_str(line->type),
				line->argc);
		for (uint32_t i = 0; i < line->argc; ++i)
		{
			printf("%s ", line->argv[i]);
		}
		putchar('\n');
	}
}

void
config_template(struct config *config,
		const char *base_dir,
		const char *src_dir)
{
	for (uint32_t i = 0; i < config->list.length; ++i)
	{
		const struct config_line *line = config->list.list[i];
		templates((struct templates) {
				.stream = NULL,
				.type = line->type,
				.argc = line->argc,
				.argv = line->argv,
				.generate_outside = false,
				.indirect_stream = NULL,
				.parent_type = TEMPLATE_NOT_FOUND,
				.parent_argc = 0,
				.parent_argv = NULL,
				.base_dir = base_dir,
				.cur_file = NULL,
				.main_file = NULL,
				.dest_dir = src_dir,
				.tscondgen = NULL
				});
	}
}

