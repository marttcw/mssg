#include "copy.h"

#include <stdio.h>
#include <string.h>

#include "hashmap.h"

struct copy_table_item {
	bool ignore;
};

static struct {
	struct hashmap	hmap;
} copy_table[COPY_FTYPE_MAX] = {
	{ .hmap = { 0 } },
	{ .hmap = { 0 } },
};

void
copy_init(void)
{
	for (uint32_t i = 0; i < COPY_FTYPE_MAX; ++i)
	{
		hashmap_create(&copy_table[i].hmap,
				16, 8, sizeof(struct copy_table_item),
				NULL, NULL);
	}
}

void
copy_deinit(void)
{
	for (uint32_t i = 0; i < COPY_FTYPE_MAX; ++i)
	{
		hashmap_destroy(&copy_table[i].hmap);
	}
}

void
copy_ignore(const char *ignore_path, const enum copy_ftype type)
{
	struct copy_table_item *item =
		hashmap_add(&copy_table[type].hmap, ignore_path);

	item->ignore = true;
}

bool
copy_ignore_this(const char *path, const enum copy_ftype type)
{
	return (hashmap_get(&copy_table[type].hmap, path) != NULL);
}

