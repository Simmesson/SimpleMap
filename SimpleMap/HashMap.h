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


// List of optimal primes stolen from GCC
static const uint32_t g_a_sizes[30] =
{
	/* 0     */              5,
	/* 1     */              11,
	/* 2     */              23,
	/* 3     */              47,
	/* 4     */              97,
	/* 5     */              199,
	/* 6     */              409,
	/* 7     */              823,
	/* 8     */              1741,
	/* 9     */              3469,
	/* 10    */              6949,
	/* 11    */              14033,
	/* 12    */              28411,
	/* 13    */              57557,
	/* 14    */              116731,
	/* 15    */              236897,
	/* 16    */              480881,
	/* 17    */              976369,
	/* 18    */              1982627,
	/* 19    */              4026031,
	/* 20    */              8175383,
	/* 21    */              16601593,
	/* 22    */              33712729,
	/* 23    */              68460391,
	/* 24    */              139022417,
	/* 25    */              282312799,
	/* 26    */              573292817,
	/* 27    */              1164186217,
	/* 28    */              2364114217,
	/* 29    */              4294967291
};