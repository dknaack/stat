#ifndef HASH_H
#define HASH_H

#define HASH_STR(str) (str), strlen(str)

typedef struct HashMap HashMap;

HashMap *hashmap_create(long size);
void     hashmap_free(HashMap *h);
void     hashmap_set(HashMap *h, const void *key, long ksize, long val);
long    *hashmap_get(HashMap *h, const void *key, long ksize);

#endif /* HASH_H */
