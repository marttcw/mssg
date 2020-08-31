#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct hashmap_item {
	char	*key;
	void 	*value;
};

struct hashmap_row {
	struct hashmap_item	*items;
	uint32_t		length;
	uint32_t		allocated;
};

struct hashmap {
	struct hashmap_row	*map;
	uint32_t		vert_length;
	uint32_t		horz_alloc_chunk;
	size_t			type_size;
	void			(* cleanup)(void *);
};

extern void hashmap_create(struct hashmap *hashmap,
		const uint32_t vert_length,
		const uint32_t horz_alloc_chunk,
		const size_t type_size,
		void (* cleanup)(void *));
extern void *hashmap_get(struct hashmap *hashmap,
		const char *key);
extern void *hashmap_add(struct hashmap *hashmap,
		const char *key);
extern void hashmap_destroy(struct hashmap *hashmap);

#define HASHMAP_STRUCT_INIT(m_vert_length, m_horz_alloc_chunk, m_type,\
		m_cleanup) { \
		.map = NULL, \
		.vert_length = m_vert_length, \
		.horz_alloc_chunk = m_horz_alloc_chunk, \
		.type_size = sizeof(m_type), \
		.cleanup = m_cleanup, \
	}

#ifdef HASHMAP_IMPLEMENTATION_H

static uint32_t
hashmap_hash(const uint32_t vert_length,
		const char *key)
{
	const uint32_t key_length = strlen(key);
	uint32_t hash = key_length;
	for (uint32_t i = 0; i < key_length; ++i)
	{
		hash += (uint32_t) key[i];
	}
	return (hash % vert_length);
}

static struct hashmap_row *
hashmap_expand(struct hashmap *hashmap,
		const uint32_t index)
{
	if (hashmap->map == NULL)
	{	// New hashmap allocation
		hashmap->map = calloc(sizeof(struct hashmap_row),
				hashmap->vert_length);
		for (uint32_t i = 0; i < hashmap->vert_length; ++i)
		{
			hashmap->map[i].items = calloc(
					sizeof(struct hashmap_item),
					hashmap->horz_alloc_chunk);
			hashmap->map[i].length = 0;
			hashmap->map[i].allocated = hashmap->horz_alloc_chunk;
			for (uint32_t j = 0;
					j < hashmap->map[i].allocated;
					++j)
			{
				hashmap->map[i].items[j].key = NULL;
				hashmap->map[i].items[j].value = NULL;
			}
		}
	}

	struct hashmap_row *row = &hashmap->map[index];
	if (row->length >= (row->allocated - 1))
	{	// Expand row
		row->allocated += hashmap->horz_alloc_chunk;
		row->items = realloc(row->items, sizeof(struct hashmap_item) *
				row->allocated);
	}
	return row;
}

void
hashmap_create(struct hashmap *hashmap,
		const uint32_t vert_length,
		const uint32_t horz_alloc_chunk,
		const size_t type_size,
		void (* cleanup)(void *))
{
	hashmap->map = NULL;
	hashmap->vert_length = vert_length;
	hashmap->horz_alloc_chunk = horz_alloc_chunk;
	hashmap->type_size = type_size;
	hashmap->cleanup = cleanup;
}

void *
hashmap_get(struct hashmap *hashmap,
		const char *key)
{
	if (hashmap->map == NULL)
	{
		return NULL;
	}

	const uint32_t hash = hashmap_hash(hashmap->vert_length, key);
	//printf("hash get: %d | key: %s\n", hash, key);
	struct hashmap_row *row = &hashmap->map[hash];
	for (uint32_t i = 0; i < row->length; ++i)
	{
		if (!strcmp(row->items[i].key, key))
		{
			return row->items[i].value;
		}
	}
	return NULL;
}

void *
hashmap_add(struct hashmap *hashmap,
		const char *key)
{
	const uint32_t key_length = strlen(key);
	const uint32_t hash = hashmap_hash(hashmap->vert_length, key);
	struct hashmap_row *row = hashmap_expand(hashmap, hash);
	const uint32_t index = row->length++;
	//printf("hash add: %d -> %d | key: %s\n", hash, index, key);

	row->items[index].key = calloc(sizeof(char), key_length + 1);
	row->items[index].value = calloc(hashmap->type_size, 1);

	strcpy(row->items[index].key, key);
	return row->items[index].value;
}

void
hashmap_destroy(struct hashmap *hashmap)
{
	if (hashmap->map == NULL)
	{
		return;
	}

	for (uint32_t i = 0; i < hashmap->vert_length; ++i)
	{
		struct hashmap_row row = hashmap->map[i];
		for (uint32_t j = 0; j < row.allocated; ++j)
		{
			struct hashmap_item item = row.items[j];
			if (item.value != NULL)
			{
				if (hashmap->cleanup != NULL)
				{
					hashmap->cleanup(item.value);
				}
				free(item.value);
			}
			if (item.key != NULL)
			{
				free(item.key);
			}
		}
		free(row.items);
	}
	free(hashmap->map);
}

#endif // HASHMAP_IMPLEMENTATION_H

#endif // HASHMAP_H

