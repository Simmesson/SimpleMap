#ifndef HASHMAP_H
#define HASHMAP_H


#include <inttypes.h>

#define HASHMAP_INSERT_RECORD_FAIL 0
#define HASHMAP_INSERT_RECORD_SUCCESS 1
#define HASHMAP_INSERT_RECORD_UPDATED 2

#define HASHMAP_DELETE_RECORD_FAIL 0
#define HASHMAP_DELETE_RECORD_SUCCESS 1

#define HASHMAP_FLAG_NONE			0x00000000
#define HASHMAP_FLAG_COPY_STRINGS	0x00000001

//! a structure to map a string to a hash
typedef struct hash_pair_struct {
	// flags for the pair
	uint32_t flags;
	// the hash of the key
	uint32_t hash;
	// the key of this pair
	const char *key;
	// the value
	void *value;
} hash_pair_t;


typedef struct hash_bucket_struct {

	hash_pair_t **pairs;

} hash_bucket_t;

typedef struct hash_map_struct {

	hash_bucket_t *pHashTable;

	uint32_t size, bucketSize;

	uint32_t count;

	int flags;

} hash_map_t;


// Performs a "benchmark" with 1 million keys
void			hashmap_benchmark();

// Creates a hash_map_t of given size and bucketSize and returns it, otherwise NULL
hash_map_t		*hashmap_create(uint32_t size, int bucketSize);

int				hashmap_insert(hash_map_t *hm, const char *key, void *data);
hash_pair_t		*hashmap_find(hash_map_t *hm, const char *key);
int				hashmap_delete(hash_map_t *hm, const char *key, int flags);

void			hashmap_free(hash_map_t *hm);



#endif // !HASHMAP_H