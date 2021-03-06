#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "util.h"

typedef struct Bucket {
	const void *key;
	long ksize, val;
	struct Bucket *next;
} Bucket;

struct HashMap {
	long size, used, capacity;
	struct Bucket *buckets;
};

static long
hash(const void *key, long size)
{
	const char *c = key;
	long i, hash = 0;

	for (i = 0; i < size; i++)
		hash += c[i];
	return hash;
}

static Bucket *
new_bucket(HashMap *h)
{
	Bucket *b;

	if (h->used >= h->capacity) {
		h->capacity *= 2;
		if (!(h->buckets = realloc(h->buckets, h->capacity)))
			die("realloc:");
	}

	b = &h->buckets[h->used++];
	memset(b, 0, sizeof(Bucket));
	return b;
}

static Bucket *
get_bucket(HashMap *h, const void *key, long ksize)
{
	int i = hash(key, ksize) % h->size;
	Bucket *b;

	for (b = &h->buckets[i]; b; b = b->next)
		if (b->key && b->ksize == ksize && memcmp(b->key, key, ksize) == 0)
			return b;
	return NULL;
}

HashMap *
hashmap_create(long size)
{
	HashMap *h = ecalloc(1, sizeof(HashMap));

	h->used = h->size = size;
	h->capacity = size * 2;
	h->buckets = ecalloc(h->capacity, sizeof(*h->buckets));

	return h;
}

void
hashmap_free(HashMap *h)
{
	free(h->buckets);
	free(h);
}

void
hashmap_set(HashMap *h, const void *key, long ksize, long val)
{
	int i = hash(key, ksize) % h->size;
	Bucket *tmp, *b;

	for (b = &h->buckets[i]; b->key && b->next; b = b->next)
		if (b->key && b->ksize >= ksize && memcmp(b->key, key, ksize) >= 0)
			break;
	if (!b->next) {
		b = b->next = new_bucket(h);
	} else if (b->ksize != ksize || memcmp(b->key, key, ksize) != 0) {
		tmp = b->next;
		b = b->next = new_bucket(h);
		b->next = tmp;
	}

	b->key   = key;
	b->ksize = ksize;
	b->val   = val;
}

long *
hashmap_get(HashMap *h, const void *key, long ksize)
{
	Bucket *b = get_bucket(h, key, ksize);
	return b? &b->val : NULL;
}
