#include "vars.h"

#include <stdio.h>
#include <stdlib.h>

#include "hashmap.h"

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

enum vars_error
vars_set(const char *name,
		const char **values,
		const uint32_t length)
{
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
	}

	if (length > 1)
	{
		var->var = realloc(var->var,
				sizeof(union var_data) * length);
	}
	var->length = length;

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
vars_loop(const char *name,
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

void
vars_loop_end(const char *name,
		const bool ended)
{
	if (ended)
	{
		hashmap_remove(&variables, name);
	}
}

