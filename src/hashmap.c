#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define STR_SIZE (256)

unsigned long get_hash(const unsigned int size, const char *key)
{
	unsigned long hash = 0;

	for (unsigned int i=0; i < strlen(key); ++i)
	{
		hash += (key[i] * i);
	}

	return hash % size;
}

hashmap *hashmap_new(const unsigned int size)
{
	hashmap *h = calloc(1, sizeof(hashmap));

	h->size = size;
	h->array = calloc(size, sizeof(kv *));
	for (unsigned int i=0; i < size; ++i) {
		h->array[i] = NULL;
	}

	return h;
}

void hashmap_destroy(hashmap *h)
{
	kv *current = NULL;
	kv *next_current = NULL;

	for (unsigned int i=0; i < h->size; ++i) {
		current = h->array[i];
		while (current != NULL) {
			if (current->key != NULL) {
				free(current->key);
			}
			if (current->value != NULL) {
				free(current->value);
			}
			next_current = current->next;
			free(current);
			current = next_current;
		}
	}
	free(h->array);
	free(h);
}

void hashmap_setValue(hashmap *h, const char *key, const char *value)
{
	unsigned int hash = get_hash(h->size, key);
	kv *current = h->array[hash];

	while (current != NULL) {
		if (!strcmp(current->key, key)) {
			strcpy(current->value, value);
			return;		// Value set, return early 
		}
		if (current->next == NULL) {
			break;
		}
		current = current->next;
	}

	// Value not found, create it
	if (current == NULL) {
		// First in the linked list
		h->array[hash] = malloc(sizeof(kv));
		current = h->array[hash];
	} else {
		// List already there
		current->next = malloc(sizeof(kv));
		current = current->next;
	}
	current->key = calloc(STR_SIZE, sizeof(char));
	current->value = calloc(STR_SIZE, sizeof(char));
	current->next = NULL;
	strcpy(current->key, key);
	strcpy(current->value, value);
}

char *hashmap_getValue(hashmap *h, const char *key)
{
	unsigned int hash = get_hash(h->size, key);
	kv *current = h->array[hash];

	for (; current != NULL; current = current->next) {
		if (!strcmp(current->key, key)) {
			return current->value;
		}
	}

	// Key not found
	return NULL;
}

void hashmap_removeKey(hashmap *h, const char *key)
{
	unsigned int hash = get_hash(h->size, key);
	kv *current = h->array[hash];
	kv *previous = NULL;

	while (current != NULL) {
		if (!strcmp(current->key, key)) {
			// Previous key's next is now this key's next
			if (previous != NULL) {
				// nth item replaced
				previous->next = current->next;
			} else {
				// root item replaced
				h->array[hash] = current->next;
			}

			// Free up this key memory
			free(current->key);
			free(current->value);
			free(current);

			return;
		}

		previous = current;
		current = current->next;
	}

	// Key not found
}

