typedef struct {
	long *keys, *vals, size;
} HashMap;

long hash_str(const char *str);

void hashmap_set(HashMap *h, long key, long val);
long hashmap_get(HashMap *h, long key);
