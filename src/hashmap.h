#ifndef HASHMAP_H
#define HASHMAP_H

typedef struct kv {
	char *key;
	char *value;
	struct kv *next;
} kv;

typedef struct {
	unsigned int size;
	kv **array;
} hashmap;

hashmap *hashmap_new(const unsigned int size);
void hashmap_destroy(hashmap *h);

void hashmap_setValue(hashmap *h, const char *key, const char *value);
char *hashmap_getValue(hashmap *h, const char *key);
void hashmap_removeKey(hashmap *h, const char *key);

#endif /* HASHMAP_H */
