#include "vars.h"

#include <stdio.h>
#include <stdlib.h>

#include "hashmap.h"
#include "generic_list.h"

enum var_type {
	VAR_TYPE_INT = 0,
	VAR_TYPE_DOUBLE,
	VAR_TYPE_STRING,
	VAR_TYPE_ERROR
};

union var_data {
	int32_t	num_int;
	double	num_double;
	char	string[256];
};

struct var {
	enum var_type	type;
	bool		settable;
	union var_data	*var;
	uint32_t	length;
};

static struct hashmap variables = { 0 };

static void
vars__cleanup(void *data)
{
	struct var *var = data;
	free(var->var);
}

static void
vars__alloc(void *data)
{
	struct var *var = data;
	var->var = calloc(sizeof(union var_data), 1);
	var->length = 1;
}

static enum var_type
vars__dettype(const char *data)
{
	const uint32_t length = strlen(data);
	uint32_t tfloat = 0;
	uint32_t tint = 0;

	for (uint32_t i = 0; i < length; ++i)
	{
		if (i > 0 && data[i] == '.')
		{
			++tfloat;
		}
		else if ('0' <= data[i] && data[i] <= '9')
		{
			++tint;
		}
		else
		{
			return VAR_TYPE_STRING;
		}
	}

	if (tfloat == 1)
	{
		return VAR_TYPE_DOUBLE;
	}
	else if (tfloat > 1)
	{
		return VAR_TYPE_ERROR;
	}
	else
	{
		return VAR_TYPE_INT;
	}
}

static enum vars_error
vars__get_int(const char *data,
		int32_t *number)
{
	const enum var_type type = vars__dettype(data);
	switch (type)
	{
	case VAR_TYPE_INT:
		*number = atoi(data);
		break;
	case VAR_TYPE_STRING:
	{
		struct var *var = hashmap_get(&variables, data);
		if (var == NULL)
		{
			fprintf(stderr, "Variable '%s' not found.\n", data);
			return VARS_ERROR_NOT_FOUND;
		}
		*number = var->var[0].num_int;
	}	break;
	default:
		return VARS_ERROR_UNSUPPORTED_TYPE;
	}

	return VARS_ERROR_NONE;
}

static void
vars__set(union var_data *dst,
		const enum var_type type,
		const union var_data src)
{
	switch (type)
	{
	case VAR_TYPE_INT:
		dst->num_int = src.num_int;
		break;
	case VAR_TYPE_DOUBLE:
		dst->num_double = src.num_double;
		break;
	case VAR_TYPE_STRING:
		strcpy(dst->string, src.string);
		break;
	default:
		break;
	}
}

void
vars_init(void)
{
	hashmap_create(&variables, 16, 8, sizeof(struct var),
			vars__cleanup,
			vars__alloc);
}

void
vars_deinit(void)
{
	hashmap_destroy(&variables);
}

struct var_inp {
	char 			*name;
	struct generic_list	values;
};

struct vinp_val {
	char	*value;
};

void
vars__varinpname_alloc(void *ptr, void *arg)
{
	struct vinp_val *value = ptr;
	uint32_t *str_size_ptr = arg;
	value->value = calloc(sizeof(char),
			(str_size_ptr == NULL) ? 256 : *str_size_ptr);
}

static void
vars__varinpname_cleanup(void *arg)
{
	struct vinp_val *value = arg;
	free(value->value);
}

static void
vars__varinp_alloc(void *ptr, void *arg)
{
	struct var_inp *vinp = ptr;
	uint32_t *name_size_ptr = arg;

	vinp->name = calloc(sizeof(char),
			(name_size_ptr == NULL) ? 256 : *name_size_ptr);

	generic_list_create(&vinp->values, 16, sizeof(struct vinp_val),
			vars__varinpname_cleanup,
			vars__varinpname_alloc);
}

static void
vars__varinp_cleanup(void *arg)
{
	struct var_inp *vinp = arg;
	free(vinp->name);
	generic_list_destroy(&vinp->values);
}

static struct generic_list *
vars__sep_multi(const char *base_name,
		const char **values,
		const uint32_t length)
{
	struct generic_list *vars = calloc(sizeof(struct generic_list), 16);
	generic_list_create(vars, 16, sizeof(struct var_inp),
			vars__varinp_cleanup,
			vars__varinp_alloc);

	int32_t cur_i = -1;
	char *cur_base[16] = { NULL };
	cur_base[0] = (char *) base_name;
	uint32_t level = 1;
	for (uint32_t i = 0; i < length; ++i)
	{
		switch (values[i][0])
		{
		case '.':
		{
			++cur_i;

			struct var_inp *vinp = generic_list_add(vars);

			cur_base[level] = (char *) values[i];
			for (uint32_t j = 0; j <= level; ++j)
			{
				strcat(vinp->name, cur_base[j]);
			}
		}	break;
		case '{':
			++level;
			cur_base[level] = (char *) values[i];
			break;
		case '}':
			--level;
			cur_base[level] = (char *) values[i];
			break;
		default:
		{
			struct var_inp *vinp = generic_list_get_last(vars);
			uint32_t len = strlen(values[i]) + 1;
			struct vinp_val *value = generic_list_add_wargs(&vinp->values,
					(void *) &len);
			strcpy(value->value, values[i]);
		}	break;
		}
	}
	return vars;
}

enum vars_error
vars_set(const char *name,
		const char **values,
		const uint32_t length)
{
	// TEST DICT
	if (values[0][0] == '.')
	{
#if 0
		for (uint32_t i = 0; i < length; ++i)
		{
			printf("\t%s\n", values[i]);
		}
		putchar('\n');
#endif

		struct generic_list *vars = vars__sep_multi(name, values, length);

		for (uint32_t i = 0; i < vars->length; ++i)
		{
			struct var_inp *vinp = generic_list_get(vars, i);

#if 0
			printf("name: %s\n", vinp->name);

			for (uint32_t j = 0; j < vinp->values.length; ++j)
			{
				printf("value: %s\n",
						(char *)
						((struct vinp_val *)
						generic_list_get(
							&vinp->values, j)
						)->value);
			}
#endif

			// TODO: vars_set
			char **values = calloc(sizeof(char *),
					vinp->values.length + 1);

			for (uint32_t j = 0; j < vinp->values.length; ++j)
			{
				values[j] = ((struct vinp_val *)
						generic_list_get(
							&vinp->values, j)
						)->value;
				//printf("values[%d]: %s\n", j, values[j]);
			}

			if (vinp->values.length > 0)
			{
				vars_set(vinp->name, (const char **) values,
						vinp->values.length);
			}

			free(values);
		}

		generic_list_destroy(vars);
		free(vars);
		return VARS_ERROR_NONE;
	}

	const enum var_type type = vars__dettype(values[0]);
	if (type == VAR_TYPE_ERROR)
	{
		return VARS_ERROR_UNSUPPORTED_TYPE;
	}

	// Find var first to override
	struct var *var = hashmap_get(&variables, name);
	if (var == NULL)
	{	// Make a new variable
		var = hashmap_add(&variables, name);
		var->settable = true;
		var->length = 1;
	}

	if (length > 1)
	{
		var->var = realloc(var->var,
				sizeof(union var_data) * length);
		var->length = length;
	}

	if (!var->settable)
	{
		return VARS_ERROR_SET_NOT_ALLOWED;
	}
	var->type = type;

	for (uint32_t i = 0; i < length; ++i)
	{
		const enum var_type dtype = vars__dettype(values[i]);

		switch (dtype)
		{
		case VAR_TYPE_INT:
			var->var[i].num_int = atoi(values[i]);
			break;
		case VAR_TYPE_DOUBLE:
			var->var[i].num_double = atof(values[i]);
			break;
		case VAR_TYPE_STRING:
			strcpy(var->var[i].string, values[i]);
			break;
		default:
			return VARS_ERROR_UNSUPPORTED_TYPE;
		}
	}

	return VARS_ERROR_NONE;
}

char *
vars_get(const char *name,
		const char *offset,
		enum vars_error *error)
{
	struct var *var = hashmap_get(&variables, name);
	if (var == NULL)
	{
		fprintf(stderr, "Variable '%s' not found.\n", name);
		*error = VARS_ERROR_NOT_FOUND;
		return NULL;
	}

	int32_t index = 0;
	if ((offset != NULL) && (var->length > 1))
	{
		enum vars_error gi_error = VARS_ERROR_NONE;
		if ((gi_error = vars__get_int(offset, &index))
				!= VARS_ERROR_NONE)
		{
			*error = gi_error;
			return NULL;
		}

		if (index >= (int32_t) var->length)
		{
			*error = VARS_ERROR_OUT_OF_RANGE;
			return NULL;
		}
	}

	static char output[1024] = { 0 };
	memset(output, 0, 1024);

	switch (var->type)
	{
	case VAR_TYPE_STRING:
		sprintf(output, "%s", var->var[index].string);
		break;
	case VAR_TYPE_INT:
		sprintf(output, "%d", var->var[index].num_int);
		break;
	case VAR_TYPE_DOUBLE:
		sprintf(output, "%f", var->var[index].num_double);
		break;
	default:
		*error = VARS_ERROR_UNSUPPORTED_TYPE;
		return NULL;
	}

	*error = VARS_ERROR_NONE;
	return output;
}

enum vars_error
vars_loop_range(const char *name,
		const char *start_str,
		const char *end_str,
		bool *ended)
{
	int32_t start_num = 0;
	int32_t end_num = 0;
	enum vars_error error = VARS_ERROR_NONE;

	if ((error = vars__get_int(start_str, &start_num)) != VARS_ERROR_NONE)
	{
		return error;
	}
	if ((error = vars__get_int(end_str, &end_num)) != VARS_ERROR_NONE)
	{
		return error;
	}

	struct var *var = hashmap_get(&variables, name);
	if (var == NULL)
	{	// First execution of loop
		var = hashmap_add(&variables, name);
		var->type = VAR_TYPE_INT;
		var->var[0].num_int = start_num;
		var->settable = false;
	}
	else
	{	// Increment/decrement
		var->var[0].num_int += (start_num <= end_num) ? 1 : -1;
	}

	*ended = (var->var[0].num_int == end_num);

	return VARS_ERROR_NONE;
}

enum vars_error
vars_loop_in(const char *name,
		const char *list,
		bool *ended)
{
	struct var *list_var = hashmap_get(&variables, list);
	if (list_var == NULL)
	{	// list not found
		*ended = true;
		return VARS_ERROR_NOT_FOUND;
	}

	char name_iter[256] = { 0 };
	sprintf(name_iter, "%s_iter", name);
	struct var *var = hashmap_get(&variables, name);
	struct var *var_iter = hashmap_get(&variables, name_iter);
	if (var == NULL && var_iter == NULL)
	{	// First execution of loop
		var = hashmap_add(&variables, name);
		var->type = list_var->type;
		var->settable = false;

		var_iter = hashmap_add(&variables, name_iter);
		var_iter->type = VAR_TYPE_INT;
		var_iter->settable = false;
		var_iter->var[0].num_int = 0;
	}

	const uint32_t index = var_iter->var[0].num_int++;

	vars__set(&var->var[0], var->type, list_var->var[index]);
	*ended = (index == (list_var->length - 1));

	return VARS_ERROR_NONE;
}

enum vars_error
vars_loop(const uint32_t argc,
		const char **argv,
		bool *ended)
{
	if (argc == 2)
	{
		return vars_loop_in(argv[0], argv[1], ended);
	}
	else
	{
		return vars_loop_range(argv[0], argv[1], argv[2], ended);
	}
}

void
vars_loop_end(const uint32_t argc,
		const char **argv,
		const bool ended)
{
	if (ended)
	{
		hashmap_remove(&variables, argv[0]);

		if (argc == 2)
		{
			char name_iter[256] = { 0 };
			sprintf(name_iter, "%s_iter", argv[0]);
			hashmap_remove(&variables, name_iter);
		}
	}
}

