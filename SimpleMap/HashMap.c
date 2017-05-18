#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

#include "HashMap.h"

/*#define DEBUG*/ // if you want debug messages with compilers without DEBUG defined

static void hashmap_rehash(hash_map_t *hm);
static void recount_bucket(hash_bucket_t *bucket, int bucketSize);

// Find prime greater than or equal to num
uint32_t prime_ge(uint32_t num) {

	for (int i = 0; i < 30; ++i) {
		if (num > g_a_sizes[i])
			continue;
		
		return g_a_sizes[i];
	}

	return NULL;
}

static uint32_t __process_buf(uint32_t hashmap_size, const uint8_t *data, const uint32_t length);

// Returns a hash of the string 'key' in the form of uint32_t based on hm->size
uint32_t process_string(uint32_t hashmap_size, const char *key) {
	if (key == NULL || hashmap_size == 0)
	{
		return NULL;
	}
	return __process_buf(hashmap_size, key, strlen(key));
}

//! Modified djb2 algorithm for hashing a buffer to an integer
static uint32_t __process_buf(uint32_t hashmap_size, const uint8_t *data, const uint32_t length)
{
	uint32_t i = 0;

	if (!data) {
		// LMERROR(strerror(EINVAL));
		return 0;
	}

	// both this number and the "33" below are special
	// they both help to make the hash values more dispersed
	// in scientific terms they increase the function's "period"
	uint32_t value = 5381;

	for (i = 0; i < length; value = (value * 33) ^ data[i++]);

	return value %= hashmap_size;
}

bool hashmap_isEmpty(hash_map_t *hm) {
	if (hm->count != 0)
		return false;

	for (uint32_t i = 0; i < hm->size; ++i)
	{
		if (!hm->pHashTable[i].pairs)
			continue;
		else return false;
	}

	return true;
}

// Returns a pointer to a newly created hashmap object
hash_map_t *hashmap_create(uint32_t size, int bucketSize)
{
	hash_map_t *map = malloc(sizeof(hash_map_t));

	if (size) {
		// Make size next prime in predefined list
		size = prime_ge(size);
	}
	else return NULL; /*size = prime_ge(STANDARD_TABLESIZE);*/

	if (size > (UINT32_MAX / bucketSize) || // Overflow
		size == 0) 
		return NULL;

#ifdef DEBUG
	printf("Selected size: %d\n", size);
#endif

	map->pHashTable = calloc(size, sizeof(hash_bucket_t));
	map->size = size;
	map->bucketSize = bucketSize;
	map->count = 0;
	map->flags = HASHMAP_FLAG_COPY_STRINGS;

	return map;
}

// Returns non-zero if success by update or creation, otherwise NULL
int hashmap_insert(hash_map_t *hm, const char *key, void *data)
{
	uint32_t	hash_index = process_string(hm->size, key),
		index = hash_index;

	int bucketIndex = 0;

	if (hm->pHashTable[index].pairs == NULL)									// Allocate bucket
	{
		hm->pHashTable[index].pairs = calloc(hm->bucketSize, sizeof(hash_pair_t*));
		memset(hm->pHashTable[index].pairs, 0, hm->bucketSize * sizeof(hash_pair_t*));
	}

	while (1)
	{
		if (hm->pHashTable[index].pairs[bucketIndex] == NULL)					// Allocate data pair
		{
			hm->pHashTable[index].pairs[bucketIndex] = malloc(sizeof(hash_pair_t));
			break;
		}

		else if (!strcmp(hm->pHashTable[index].pairs[bucketIndex]->key, key))	// Update existing record
		{
			hm->pHashTable[index].pairs[bucketIndex]->value = data;
			return HASHMAP_INSERT_RECORD_UPDATED;
		}

		else
		{
			++bucketIndex;
			bucketIndex %= hm->bucketSize;

			if (bucketIndex == 0) // No space left, rehash
			{
				uint32_t oldSize = hm->size;
				hashmap_rehash(hm);
				if (oldSize == hm->size)
					return HASHMAP_INSERT_RECORD_FAIL;
#ifdef DEBUG
				printf("Grew to %ld\n", hm->size);
#endif
				return hashmap_insert(hm, key, data);				
			}
		}
	}

	hm->pHashTable[index].pairs[bucketIndex]->flags = 1;
	hm->pHashTable[index].pairs[bucketIndex]->value = data;
	hm->pHashTable[index].pairs[bucketIndex]->hash = hash_index;

	if (hm->flags == HASHMAP_FLAG_COPY_STRINGS) 
	{
		hm->pHashTable[index].pairs[bucketIndex]->key = malloc(strlen(key) + 1);
		strcpy(hm->pHashTable[index].pairs[bucketIndex]->key, key);
	}
	else 
	{
		hm->pHashTable[index].pairs[bucketIndex]->key = key;
	}

	++(hm->count);

	return HASHMAP_INSERT_RECORD_SUCCESS;
}

// Returns the hash pair by the given key, or NULL if not found.
hash_pair_t *hashmap_find(hash_map_t *hm, const char *key)
{
	uint32_t index = process_string(hm->size, key);
	int bucketIndex = 0;

	if (hm->pHashTable[index].pairs == NULL)
		return NULL;

	while (hm->pHashTable[index].pairs[bucketIndex] != NULL)
	{

		if (!strcmp(hm->pHashTable[index].pairs[bucketIndex]->key, key))
			return hm->pHashTable[index].pairs[bucketIndex];

		++bucketIndex;
		bucketIndex %= hm->bucketSize;

		if (bucketIndex == 0)
			return NULL;
	}

	return NULL;
}

static void hashmap_rehash(hash_map_t *hm) 
{
	uint32_t newSize = prime_ge(hm->size + 1);
	int newBucketSize = hm->bucketSize;

	hash_bucket_t *newMap = (hash_bucket_t*)calloc(newBucketSize, newSize);

	for (uint32_t i = 0; i < hm->size; ++i) 
	{
		while (hm->pHashTable[i].pairs)
		{
			if (!hm->pHashTable[i].pairs[0]) { // Done with this bucket
				free(hm->pHashTable[i].pairs);
				hm->pHashTable[i].pairs = NULL;
				break;
			}

			uint32_t index = process_string(newSize, hm->pHashTable[i].pairs[0]->key);
			int bucketIndex = 0;

			if (!newMap[index].pairs)
			{
				newMap[index].pairs = calloc(newBucketSize, sizeof(hash_pair_t*));
			}

			do 
			{
				if (!newMap[index].pairs[bucketIndex]) // Allocate and then copy from prev map
				{
					newMap[index].pairs[bucketIndex] = malloc(sizeof(hash_pair_t));

					*(newMap[index].pairs[bucketIndex]) = *(hm->pHashTable[i].pairs[0]);
					newMap[index].pairs[bucketIndex]->hash = index;

					free(hm->pHashTable[i].pairs[0]);
					hm->pHashTable[i].pairs[0] = NULL;

					recount_bucket(&hm->pHashTable[i], hm->bucketSize);

					break;
				}

				++bucketIndex;
				bucketIndex %= newBucketSize;

			} while (bucketIndex != 0);

		}
	}

	hm->size = newSize;
	hm->bucketSize = newBucketSize;
	free(hm->pHashTable);
	hm->pHashTable = newMap;
}

void hashmap_empty(hash_map_t *hm)
{
	for (uint32_t i = 0; i < hm->size; ++i)
	{
		if (!hm->pHashTable[i].pairs)
			continue;

		else 
		{
			for (int j = 0; j < hm->bucketSize; ++j)
			{
				if (hm->pHashTable[i].pairs[j])
				{
					free(hm->pHashTable[i].pairs[j]);
					/*hm->pHashTable[i].pairs[j] = NULL;*/ // No need for NULL assignment when hashmap is gonna be totally emptied
				}
			}
			free(hm->pHashTable[i].pairs);
			hm->pHashTable[i].pairs = NULL; // Assign this though; could get used again
		}
		
	}
}

void hashmap_free(hash_map_t *hm)
{
	if (!hashmap_isEmpty(hm))
	{
		hashmap_empty(hm);
	}
	free(hm->pHashTable);
	free(hm);
	hm->pHashTable = NULL;
	hm = NULL;
}

static void recount_bucket(hash_bucket_t *bucket, int bucketSize) 
{
	bool freeSpot = false;
	for (int bucketindex = 0; bucketindex < bucketSize; ++bucketindex)
	{
		if (bucket->pairs[bucketindex] != NULL) 
		{
			for (int j = bucketindex - 1; j >= 0 && freeSpot; --j)
			{
				if (bucket->pairs[j])
					continue;

				bucket->pairs[j] = bucket->pairs[bucketindex]; // Place in first available spot from bottom
				bucket->pairs[bucketindex] = NULL;
				break;
			}			
		}
		else freeSpot = true;
	}

	if (bucket->pairs[0] == NULL)
	{
		free(bucket->pairs);
		bucket->pairs = NULL;
	}
}

// Returns NULL if unsuccessful, otherwise HASHMAP_DELETE_RECORD_SUCCESS
int hashmap_delete(hash_map_t *hm, const char *key, int flags) {
	uint32_t index = process_string(hm->size, key);

	if (hm->pHashTable[index].pairs == NULL)
		goto END;

	int bucketIndex = 0;
	do {
		if (hm->pHashTable[index].pairs[bucketIndex] != NULL &&
			!strcmp(hm->pHashTable[index].pairs[bucketIndex]->key, key)) {
			if (flags == 1) // Should delete strings
			{
				free(hm->pHashTable[index].pairs[bucketIndex]->key);
				hm->pHashTable[index].pairs[bucketIndex]->key = NULL;
			}
			free(hm->pHashTable[index].pairs[bucketIndex]);
			hm->pHashTable[index].pairs[bucketIndex] = NULL;
			hm->count -= 1;
			recount_bucket(&hm->pHashTable[index], hm->bucketSize);
			return HASHMAP_DELETE_RECORD_SUCCESS;
		}

		++bucketIndex;
		bucketIndex %= hm->bucketSize;
	} while (bucketIndex != 0);

END:
#ifdef DEBUG
	printf("Memory error: Couldn\'t find key \"%s\".", key);
#endif
	return HASHMAP_DELETE_RECORD_FAIL;
}

#define TEST_KEY_LEN    25
#define MAX_TESTS       1000000

// the character set for key generation
#define CHARSET_START   'a'
#define CHARSET_END     'z'
#define CHARSET         ((CHARSET_END - CHARSET_START) + 1)

//! generate a list of unique keys
uint32_t gen_key_list(hash_map_t *HashMap)
{
	clock_t start, end;
	double cpu_time_used;
	uint32_t test_amount = 0;
	uint32_t index = 0;
	uint32_t i = 0;
	uint32_t j = 0;

	int mode = 0;

	char base_string[TEST_KEY_LEN + 1] = { 0 };
	memset(base_string, CHARSET_START, TEST_KEY_LEN);

	// permutations = pow(number_of_letters, string_length);
	// this will do this: max(pow(CHARSET, TEST_KEY_LEN), MAX_TESTS)
	// either the maximum permutations for this charset or the
	// MAX_TESTS amount will be chosen as test_amount
	for (test_amount = CHARSET, i = 0;
		i < TEST_KEY_LEN;
		(((test_amount * CHARSET) < MAX_TESTS)
			? (test_amount *= CHARSET)
			: (test_amount = MAX_TESTS)),
		i++);

	// do this all 4 times
	while (mode < 3) {
		memset(base_string, CHARSET_START, TEST_KEY_LEN);
		// start timer or something, print starting mode %d etc
		printf("Starting mode %d\n", mode);
		start = clock();

		for (i = 0; i < test_amount; ++i) {
			base_string[index]++;
			if (base_string[index] == 'z') {
				for (j = 0; j < TEST_KEY_LEN; ++j) {
					if (base_string[index + j] != 'z') {
						base_string[index + j]++;
						break;
					}
					else {
						base_string[index + j] = 'a';
					}
				}
				base_string[index] = 'a';
			}
			index = index % TEST_KEY_LEN;

			int test;
			switch (mode) {
			case 0: // store
				assert(hashmap_insert(HashMap, base_string, base_string));
				break;
			case 1: // fetch
				assert(hashmap_find(HashMap, base_string));
				break;
			case 2: // delete
				assert(hashmap_delete(HashMap, base_string, 1));
				break;
			}
		}

		end = clock();
		mode++;
		printf("Done mode %d after %f seconds\n\n", mode - 1, (double)(end - start) / CLOCKS_PER_SEC);
	}

#ifdef DEBUG
	hashmap_isEmpty(HashMap) ? printf("Inserted, found and deleted %d keys.\n", i) : printf("Couldn\'t delete them all..\n");
#endif
	
	return i;
}

void hashmap_benchmark()
{
	hash_map_t *HashMap = hashmap_create(1000 * 1000, 13);

	gen_key_list(HashMap);

	hashmap_free(HashMap);
}