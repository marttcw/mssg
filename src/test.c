#include <stdio.h>

#define HASHMAP_IMPLEMENTATION_H
#include "hashmap.h"

struct test {
	uint32_t 	number;
	char		*value;
};

void
test_cleanup(void *data)
{
	struct test *test = data;
	if (test->value != NULL)
	{
		free(test->value);
	}
}

int
main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	struct hashmap hashmap = {
		.map = NULL,
		.vert_length = 10,
		.horz_alloc_chunk = 8,
		.type_size = sizeof(struct test),
		.cleanup = test_cleanup,
	};

	struct test *test = hashmap_add(&hashmap, "sneed");
	test->number = 0;
	test->value = calloc(sizeof(char), 15);
	strcpy(test->value, "sneed 0");

	test = hashmap_add(&hashmap, "feed");
	test->number = 5;
	test->value = calloc(sizeof(char), 25);
	strcpy(test->value, "feedofeed");

	struct test *get = hashmap_get(&hashmap, "sneed");
	printf("%d %s\n", get->number, get->value);

	get = hashmap_get(&hashmap, "feed");
	printf("%d %s\n", get->number, get->value);

	get = hashmap_get(&hashmap, "never");
	printf("%d\n", get);

	hashmap_destroy(&hashmap);

	return 0;
}

