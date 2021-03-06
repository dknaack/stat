#include <string.h>
#include <stdio.h>

#include "hash.h"

long
hash_str(const char *str)
{
	int i, len;
	long hash = 0;

	if (!str)
		return 0;
	len = strlen(str);
	for (i = 0; i < len; i++)
		hash += str[i];

	return hash;
}

void
hashmap_set(HashMap *h, long key, long val)
{
	h->keys[key % h->size] = key;
	h->vals[key % h->size] = val;
}

long
hashmap_get(HashMap *h, long key)
{
	return h->vals[key % h->size];
}
