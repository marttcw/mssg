#ifndef GENERIC_LIST_H
#define GENERIC_LIST_H

struct generic_list {
	void		 	**list;
	uint32_t		length;
	uint32_t		allocated;
	const uint32_t		ALLOC_CHUNK;
	const size_t		type_size;
	const bool		is_pointer;
	void			(* cleanup)(void *);
};

static void
generic_list_expand(struct generic_list *generic_list)
{
	if (generic_list->list == NULL)
	{
		generic_list->allocated = generic_list->ALLOC_CHUNK;
		generic_list->list = calloc(
				sizeof(void *), generic_list->allocated);
		for (uint32_t i = 0; i < generic_list->allocated; ++i)
		{
			generic_list->list[i] = calloc(
					generic_list->type_size, 1);
		}
	}
	else if (generic_list->length == (generic_list->allocated - 1))
	{
		const uint32_t i_start = generic_list->allocated;
		generic_list->allocated += generic_list->ALLOC_CHUNK;
		generic_list->list = realloc(generic_list->list,
				generic_list->type_size *
					generic_list->allocated
				);
		for (uint32_t i = i_start; i < generic_list->allocated; ++i)
		{
			generic_list->list[i] = calloc(
					generic_list->type_size, 1);
		}
	}
}

static void *
generic_list_get(struct generic_list *generic_list,
		const uint32_t index)
{
	return generic_list->list[index];
}

static void *
generic_list_add(struct generic_list *generic_list)
{
	generic_list_expand(generic_list);
	return generic_list->list[generic_list->length++];
}

static void
generic_list_destroy(struct generic_list *generic_list)
{
	for (uint32_t i = 0; i < generic_list->allocated; ++i)
	{
		if (generic_list->cleanup != NULL)
		{
			generic_list->cleanup(generic_list->list[i]);
		}
		free(generic_list->list[i]);
	}
	free(generic_list->list);
	if (generic_list->is_pointer)
	{
		free(generic_list);
	}
	generic_list = NULL;
}

#endif // GENERIC_LIST_H

